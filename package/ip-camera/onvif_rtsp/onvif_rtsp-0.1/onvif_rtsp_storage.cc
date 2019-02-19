#include "onvif_rtsp_storage.h"
#include <ctime>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <iomanip>
#include <iostream>
#include <regex>
#include <list>

static std::list<std::string> gs_file_list;

static long
storage_get_available_space (std::string location) {
  struct statvfs stat;
  if (statvfs (location.c_str (), &stat) != 0) {
    return -1;
  }

  return stat.f_bsize * stat.f_bavail;
}

static void
storage_load_filelist (std::string location) {
  DIR *dir = opendir (location.c_str ());
  struct dirent *d;

  gs_file_list.clear ();
  if (dir) {
    while ((d = readdir (dir)) != NULL) {
      if (d->d_type != DT_REG) {
        continue;
      }
      char *fn = &d->d_name[0];
      std::regex e ("^\\d{4}-\\d{2}-\\d{2}_\\d{2}-\\d{2}-\\d{2}\\.mp4$");
      if (std::regex_search (fn, e)) {
        gs_file_list.push_back (fn);
      }
    }
    closedir (dir);
  }
  gs_file_list.sort ();

  return;
}

static void
storage_freeup_space (std::string location, long reserved_space_size) {
  long valid_space;

  while ((valid_space = storage_get_available_space (location)) < reserved_space_size &&
      gs_file_list.size () > 1) {
    std::string filename_remove = location + "/" + gs_file_list.front ();

    unlink (filename_remove.c_str ());

    g_print ("removing %s ... \n  free space %d\n",
        filename_remove.c_str (), valid_space);

    gs_file_list.pop_front ();

  }

}

static std::string
storage_build_filename (void) {
  std::time_t t = std::time (nullptr);
  char time_str[64];
  std::strftime (time_str, sizeof (time_str), "%Y-%m-%d_%H-%M-%S", std::localtime (&t));
  return std::string (time_str) + ".mp4";
}

static void
storage_set_filelocation (RTSP_SERVER_t *srv) {
  PIPELINE_STO_t *sto = &srv->pipelines.sto;
  std::shared_ptr<RTSP_CONFIG_t> config = srv->config;

  std::string filename = "/dev/null";

  if (config->storage.enabled) {
    // check dir existance
    DIR *dir = opendir (config->storage.location.c_str ());
    if (!dir) {
      if (mkdir (config->storage.location.c_str (), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        perror ("create storage dir");
      }
    }
    closedir (dir);
    filename = storage_build_filename ();
    gs_file_list.push_back (filename);

    filename = config->storage.location + "/" + filename;
  }

  g_object_set (G_OBJECT (sto->filesink),
      "location", filename.c_str (),
      NULL);
  g_print ("saving file to %s\n", filename.c_str ());
}

static gboolean
on_switch_file (RTSP_SERVER_t *srv) {
  PIPELINE_STO_t *sto = &srv->pipelines.sto;
  std::shared_ptr<RTSP_CONFIG_t> config = srv->config;
  if (sto->pipeline == NULL) {
    fprintf (stderr, "In the switch file callback the user data was NULL, fatal!\n");

    /* quit loop to exit application */
    if (srv->loop != NULL) {
      g_main_loop_quit (srv->loop);
    }

    return GST_PAD_PROBE_REMOVE;
  }

  /* set the state of the muxer and filesink element
     to NULL, causing everything to be reset to the initial
     state (and handles closed etc) */
  gst_element_set_state (sto->filesink, GST_STATE_NULL);
  gst_element_set_state (sto->muxer, GST_STATE_NULL);

  /* generate a new filename with the current date/time and
     apply it on the file sink */
  storage_set_filelocation (srv);

  /* sync state of the muxer and filesink with the parent,
     which is PLAYING, causing everything to resume again */
  gst_element_set_state (sto->muxer, GST_STATE_PLAYING);
  gst_element_set_state (sto->filesink, GST_STATE_PLAYING);

  /* remove the blocking buffer probe so frames can once
     again pass through the pipeline */
  gst_pad_remove_probe (sto->parser_src_pad, GST_PAD_PROBE_INFO_ID (sto->vbuffer_probe));
  sto->vbuffer_probe = NULL;

  /* set flag to false again, so that we can switch once again */
  sto->is_switching = false;

  if (!config->debug.disable_audio) {
    gst_pad_remove_probe (sto->adepay_src_pad, GST_PAD_PROBE_INFO_ID (sto->abuffer_probe));
    sto->is_audio_pad_blocked = false;
    sto->abuffer_probe = NULL;
  }

  /* returning FALSE is really important, otherwise the main loop
     will execute this function over and over again */
  return FALSE;
}

static GstPadProbeReturn
on_filesink_eos (GstPad *pad, GstPadProbeInfo *info, RTSP_SERVER_t *srv) {
  /* make sure we actually received EOS and not another
     event or a buffer, if it's not EOS, pass because it
     might be buffers that are written to the file at the
     last moment */

  if (!GST_IS_EVENT (info->data)) {
    return GST_PAD_PROBE_PASS;
  }

  GstEvent *event = GST_EVENT (info->data);
  GstEventType event_type = GST_EVENT_TYPE (event);

  if (event_type != GST_EVENT_EOS) {
    return GST_PAD_PROBE_PASS;
  }

  /* remove the probe, preventing us from intercepting
     more events */
  gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

  /* perform the actual switch on the application thread
     as opposed to the streaming thread */
  g_idle_add ((GSourceFunc) on_switch_file, (gpointer) srv);

  /* very important that we drop, otherwise, the EOS event will
     reach the end of the pipeline, thus bringing the whole pipeline down */
  return GST_PAD_PROBE_DROP;
}

static GstPadProbeReturn
on_buffer_probe (GstPad *pad, GstPadProbeInfo *info, RTSP_SERVER_t *srv)
{
  PIPELINE_STO_t *sto = &srv->pipelines.sto;
  std::shared_ptr<RTSP_CONFIG_t> config = srv->config;
  if (sto->pipeline == NULL) {
    fprintf (stderr, "In buffer probe callback the user data was NULL, fatal!\n");

    /* quit loop to exit application */
    if (srv->loop != NULL) {
      g_main_loop_quit (srv->loop);
    }

    return GST_PAD_PROBE_REMOVE;
  }

  if (!config->debug.disable_audio) {
    if (pad == sto->adepay_src_pad) {
      sto->is_audio_pad_blocked = true;
      sto->abuffer_probe = info;
      gst_pad_send_event (sto->muxer_asink_pad, gst_event_new_eos ());
      return GST_PAD_PROBE_OK;
    } else {
      if (!sto->is_audio_pad_blocked) {
        // wait for audio pad blocked
        return GST_PAD_PROBE_PASS;
      }
    }
  }

  GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER (info);

  /* is this frame a key frame? */
  if (GST_BUFFER_FLAG_IS_SET (buffer, GST_BUFFER_FLAG_DELTA_UNIT)) {
    /* not a key frame, in case we have not set buffer_probe,
       we are still waiting for a key frame, let the curent frame
       pass. if not, block that damn thing */
    if (sto->vbuffer_probe == NULL) {
      return GST_PAD_PROBE_PASS;
    } else {
      return GST_PAD_PROBE_OK;
    }
  }

  /* setting so this probe can be removed when the
     switch is done */
  sto->vbuffer_probe = info;

  /* add an event probe (that is blocking) on the sink pad of the filesink
     so when we sent eos through the muxer, we'll know we receieved it */
  gst_pad_add_probe (sto->filesink_sink_pad,
      (GstPadProbeType) (GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_EVENT_BOTH),
      (GstPadProbeCallback) on_filesink_eos, srv, NULL);

  /* send eos through the muxer, causing it to correctly write everything
     to the file */
  gst_pad_send_event (sto->muxer_vsink_pad, gst_event_new_eos ());

  /* continue blocking */
  return GST_PAD_PROBE_OK;
}

static gboolean
on_timeout (RTSP_SERVER_t *srv)
{
  PIPELINE_STO_t *sto = &srv->pipelines.sto;
  std::shared_ptr<RTSP_CONFIG_t> config = srv->config;
  if (sto->pipeline == NULL) {
    fprintf (stderr, "In timeout callback the user data was NULL, fatal!\n");

    /* quit loop to exit application */
    if (srv->loop != NULL) {
      g_main_loop_quit (srv->loop);
    }

    return G_SOURCE_REMOVE;
  }

  storage_freeup_space (config->storage.location,
      config->storage.reserved_space_size << 20);

  /* do not start another switch if still switching */
  if (sto->is_switching) {
    return G_SOURCE_CONTINUE;
  }

  /* in case of short timeouts, it can happen that the timeout is called
     before we're done switching, set flag to prevent that */
  sto->is_switching = true;

  /* add a blocking probe to wait for a key frame, when we
     encounter a key frame, we'll do the switch, this way the
     first frame in the next file is a key frame */
  gst_pad_add_probe (sto->parser_src_pad,
      (GstPadProbeType) (GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BLOCK),
      (GstPadProbeCallback) on_buffer_probe, srv, NULL);

  if (!config->debug.disable_audio) {
    /* add a blocking probe to audio pad */
    gst_pad_add_probe (sto->adepay_src_pad,
        (GstPadProbeType) (GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BLOCK),
        (GstPadProbeCallback) on_buffer_probe, srv, NULL);
  }

  return G_SOURCE_CONTINUE;
}

static bool
storage_init (RTSP_SERVER_t *srv) {
  GError *error = NULL;
  std::shared_ptr<RTSP_CONFIG_t> config = srv->config;

  PIPELINE_STO_t *sto = &srv->pipelines.sto;

  std::string pipeline_desc = pipeline_create_sto (config);

  sto->is_running = false;
  sto->is_switching = false;
  if (!config->debug.disable_audio) {
    sto->is_audio_pad_blocked = false;
  }
  sto->pipeline = gst_parse_launch (pipeline_desc.c_str (), &error);

  sto->vsrc = gst_bin_get_by_name (GST_BIN (sto->pipeline), "vsrc");
  if (!config->debug.disable_audio) {
    sto->asrc = gst_bin_get_by_name (GST_BIN (sto->pipeline), "asrc");
    sto->adepay = gst_bin_get_by_name (GST_BIN (sto->pipeline), "adepay");
  }
  sto->parser = gst_bin_get_by_name (GST_BIN (sto->pipeline), "parser");
  sto->muxer = gst_bin_get_by_name (GST_BIN (sto->pipeline), "muxer");
  sto->filesink = gst_bin_get_by_name (GST_BIN (sto->pipeline), "recordfile");


  storage_set_filelocation (srv);

  /* get pads of some elements that we're going to use during
	 the switching of files */
  sto->parser_src_pad = gst_element_get_static_pad (sto->parser, "src");
  sto->muxer_vsink_pad = gst_element_get_static_pad (sto->muxer, "video_0");
  if (!config->debug.disable_audio) {
    sto->adepay_src_pad = gst_element_get_static_pad (sto->adepay, "src");
    sto->muxer_asink_pad = gst_element_get_static_pad (sto->muxer, "audio_0");
  }
  sto->filesink_sink_pad = gst_element_get_static_pad (sto->filesink, "sink");

  return true;
}

#define GST_OBJ_FREE(e) do {\
    if (e) { \
      g_object_unref (e); \
      e = NULL; \
    } \
  } while (0)

static void
storage_uninit (RTSP_SERVER_t *srv) {
  PIPELINE_STO_t *sto = &srv->pipelines.sto;
  std::shared_ptr<RTSP_CONFIG_t> config = srv->config;

  GST_OBJ_FREE (sto->pipeline);

  GST_OBJ_FREE (sto->vsrc);
  if (!config->debug.disable_audio) {
    GST_OBJ_FREE (sto->asrc);
    GST_OBJ_FREE (sto->adepay);
  }
  GST_OBJ_FREE (sto->parser);
  GST_OBJ_FREE (sto->muxer);
  GST_OBJ_FREE (sto->filesink);

  /* get pads of some elements that we're going to use during
	 the switching of files */
  GST_OBJ_FREE (sto->parser_src_pad);
  GST_OBJ_FREE (sto->muxer_vsink_pad);
  if (!config->debug.disable_audio) {
    GST_OBJ_FREE (sto->adepay_src_pad);
    GST_OBJ_FREE (sto->muxer_asink_pad);
  }
  GST_OBJ_FREE (sto->filesink_sink_pad);
}

static gboolean
delay_start_storage (RTSP_SERVER_t *srv) {
  PIPELINE_SRC_t *src = &srv->pipelines.src;
  PIPELINE_STO_t *sto = &srv->pipelines.sto;
  std::shared_ptr<RTSP_CONFIG_t> config = srv->config;

  if (src->vsink_caps == NULL ||
      (!config->debug.disable_audio && src->asink_caps == NULL)){
    g_print ("waiting for caps ...");
    return TRUE; // return true to keep the callback retry again
  }

  // start storage pipeline until the caps got
  g_object_set (G_OBJECT (sto->vsrc),
      "caps", src->vsink_caps,
      NULL);

  if (!config->debug.disable_audio) {
    g_object_set (G_OBJECT (sto->asrc),
        "caps", src->asink_caps,
        NULL);
  }

  /* start playing the pipeline, causes recording to start */
  gst_element_set_state (sto->pipeline, GST_STATE_PLAYING);

  /* using get_state we wait for the state change to complete */
  if (gst_element_get_state (sto->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE)
	  == GST_STATE_CHANGE_FAILURE) {
	g_print ("Failed to get the pipeline into the PLAYING state\n");
    gst_element_set_state (sto->pipeline, GST_STATE_NULL);
    return FALSE;
  }

  sto->is_running = true;

  guint interval = std::stoi (config->storage.chunk_duration) * 60;

  if (interval > 0) {
    g_print ("media chunk file length: %d minutes\n", interval / 60);
    g_timeout_add_seconds (interval, (GSourceFunc) on_timeout, (gpointer) srv);
  }

  g_print ("storage pipeline activated ...\n");
  return FALSE;
}

bool rtsp_storage_start (RTSP_SERVER_t *srv) {
  PIPELINE_STO_t *sto = &srv->pipelines.sto;
  std::shared_ptr<RTSP_CONFIG_t> config = srv->config;

  if (!config->storage.enabled) {
    g_print ("Storage disabled\n");
    return true;
  }

  if (sto->pipeline == NULL) {
    if (!storage_init (srv)) {
      return false;
    }
  }

  if (sto->is_running == true) {
    return true;
  }

  storage_load_filelist (config->storage.location);
  storage_freeup_space (config->storage.location,
      config->storage.reserved_space_size << 20);


  g_timeout_add (10, (GSourceFunc) delay_start_storage, (gpointer) srv);

  return true;
}

bool rtsp_storage_stop (RTSP_SERVER_t *srv) {
  PIPELINE_STO_t *sto = &srv->pipelines.sto;
  if (sto->pipeline == NULL) {
    return true;
  }

  /* terminating, set pipeline to NULL and clean up */
  g_print ("Closing storage pipeline\n");

  gst_element_set_state (sto->pipeline, GST_STATE_NULL);
  if (gst_element_get_state (sto->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE)
      == GST_STATE_CHANGE_FAILURE) {
    g_print ("Failed to get the pipline into the NULL state\n");
    return false;
  }
  sto->is_running = false;
  storage_uninit (srv);
  return true;
}
