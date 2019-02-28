#include "onvif_rtsp_source.h"
#include "onvif_rtsp_v4l2ctl.h"
#include "onvif_rtsp_common.h"

static void
on_v4l2src_prepare_format (GstElement* object, gint fd, GstCaps* caps, RTSP_SERVER_t *srv) {
  std::shared_ptr<CONFIG_t> config = srv->config;

  // set fps
  onvif_rtsp_v4l2ctl_fps (fd, std::stoi(config->video.framerate));
  // set wdr mode
  onvif_rtsp_v4l2ctl_wdr (fd, config->isp.wdr_enabled);

}

static void
on_notify_caps (GstPad *pad, GParamSpec *pspec, GstCaps **pcaps) {
  GstCaps *caps;
  g_object_get (pad, "caps", &caps, NULL);

  if (caps) {
    if (*pcaps) {
      gst_caps_unref (*pcaps);
    }
    *pcaps = gst_caps_copy (caps);
  }

}

#define GST_EVENT_CAPIMG GST_EVENT_MAKE_TYPE(81, GST_EVENT_TYPE_DOWNSTREAM | GST_EVENT_TYPE_SERIALIZED)

static void
do_capture_image (RTSP_SERVER_t *srv) {
  std::shared_ptr<CONFIG_t> config = srv->config;

  if (!config->imagecap.enabled || config->imagecap.location.empty()) return;

  PIPELINE_SRC_t *src = &srv->pipelines.src;

  rtsp_common_create_dir (config->imagecap.location.c_str());
  std::string filename = rtsp_common_build_filename(".jpg");

  GstStructure *resst = gst_structure_new ("capture-image",
      "location", G_TYPE_STRING, filename.c_str(),
      NULL);

  GstEvent *capimg_event = gst_event_new_custom ((GstEventType)GST_EVENT_CAPIMG,
      resst);
  gst_element_send_event (src->imgcap, capimg_event);

}

static bool source_init (RTSP_SERVER_t *srv) {
  GError *error = NULL;
  std::shared_ptr<CONFIG_t> config = srv->config;

  std::string pipeline_desc = pipeline_create_src (config);
  PIPELINE_SRC_t *src = &srv->pipelines.src;

  src->pipeline = gst_parse_launch (pipeline_desc.c_str (), &error);
  src->vsrc = gst_bin_get_by_name (GST_BIN (src->pipeline), "vsrc");
  src->vsink = gst_bin_get_by_name (GST_BIN (src->pipeline), "vsink");
  if (config->imagecap.enabled) {
    src->imgcap = gst_bin_get_by_name (GST_BIN (src->pipeline), "imgcap");
  }
  src->vsink_sink_pad = gst_element_get_static_pad (src->vsink, "sink");
  src->vsink_caps = NULL;
  g_signal_connect (src->vsink_sink_pad, "notify::caps",
      G_CALLBACK (on_notify_caps), &src->vsink_caps);

  if (!config->debug.disable_audio) {
    src->asink = gst_bin_get_by_name (GST_BIN (src->pipeline), "asink");
    src->asink_sink_pad = gst_element_get_static_pad (src->asink, "sink");
    src->asink_caps = NULL;
    g_signal_connect (src->asink_sink_pad, "notify::caps",
        G_CALLBACK (on_notify_caps), &src->asink_caps);
  }

  g_signal_connect (src->vsrc, "prepare-format",
      G_CALLBACK (on_v4l2src_prepare_format), srv);

  return true;
}

bool rtsp_source_start (RTSP_SERVER_t *srv) {
  PIPELINE_SRC_t *src = &srv->pipelines.src;
  if (src->pipeline == NULL) {
    if (!source_init (srv)) {
      return false;
    }
  }
  /* start playing the pipeline, causes recording to start */
  gst_element_set_state (src->pipeline, GST_STATE_PLAYING);

  /* using get_state we wait for the state change to complete */
  if (gst_element_get_state (src->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE)
	  == GST_STATE_CHANGE_FAILURE) {
	g_print ("Failed to get the pipeline into the PLAYING state\n");
    gst_element_set_state (src->pipeline, GST_STATE_NULL);
    return false;
  }

  return true;


}

bool rtsp_source_stop (RTSP_SERVER_t *srv) {
  PIPELINE_SRC_t *src= &srv->pipelines.src;
  if (src->pipeline == NULL) {
    return true;
  }

  /* terminating, set pipeline to NULL and clean up */
  g_print ("Closing stream and file\n");

  gst_element_set_state (src->pipeline, GST_STATE_NULL);
  if (gst_element_get_state (src->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE)
      == GST_STATE_CHANGE_FAILURE) {
    g_print ("Failed to get the pipline into the NULL state\n");
    return false;
  }
  return true;
}
