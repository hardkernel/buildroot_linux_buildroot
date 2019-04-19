/*
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2018 Jemy Zhang <<jun.zhang@amlogic.com>>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-amlfacenet
 *
 * FIXME:Describe amlfacenet here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amlfacenet ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/base.h>
#include <gst/video/video.h>
#include <gst/controller/controller.h>
#include <gmodule.h>

#include "gstamlfacenet.h"
#include "facenetdb.h"
#include "framecrop.h"

GST_DEBUG_CATEGORY_STATIC (gst_aml_facenet_debug);
#define GST_CAT_DEFAULT gst_aml_facenet_debug

struct RelativePos {
  float x0;
  float y0;
  float x1;
  float y1;
};

struct FaceInput {
  struct listnode list;
  guint64 frameidx;
  struct RelativePos pos;
  char *faceimg;
};

struct FaceNetInput {
  struct listnode list;
  guint64 frameidx;
  struct RelativePos pos;
  char *faceimg;
};

struct FaceNetResult {
  struct listnode list;
  guint64 frameidx;
  struct RelativePos pos;
  char *faceimg;
  float result[128];
};

struct FaceNetDBResult {
  guint64 frameidx;
  struct RelativePos pos;
  char *info;
};

struct NNResult {
  struct listnode list;
  guint64 frameidx;
  int  detect_num;
  struct RelativePos *pt;
};


#define DEFAULT_PROP_MODEL_TYPE DET_FACENET
#define DEFAULT_PROP_FORMAT "uid,name"
#define DEFAULT_PROP_DBPATH ""
#define DEFAULT_PROP_STORE_FACE FALSE
#define DEFAULT_PROP_THRESHOLD 0.6

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_DBPATH,
  PROP_FORMAT,
  PROP_THRESHOLD,
  PROP_STORE_FACE,
};

/* the capabilities of the inputs and outputs.
 *
 * FIXME:describe the real formats here.
 */
static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE (
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
      "framerate = (fraction) [0/1, MAX], "
      "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ], "
      "format = (string) { RGB } ")
    );

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE (
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("video/x-raw, "
      "framerate = (fraction) [0/1, MAX], "
      "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ], "
      "format = (string) { RGB } ")
);

#define gst_aml_facenet_parent_class parent_class
G_DEFINE_TYPE (GstAmlFacenet, gst_aml_facenet, GST_TYPE_BASE_TRANSFORM);

static void gst_aml_facenet_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_aml_facenet_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_aml_facenet_open (GstBaseTransform * base);

static gboolean gst_aml_facenet_close (GstBaseTransform * base);

static void gst_aml_facenet_finalize (GObject * object);

static GstFlowReturn gst_aml_facenet_transform_ip (GstBaseTransform * base,
    GstBuffer * outbuf);

static gboolean gst_aml_facenet_set_caps (GstBaseTransform * base,
    GstCaps * incaps, GstCaps * outcaps);

static gboolean gst_aml_facenet_event(GstBaseTransform * base, GstEvent *event);

static void cleanup_nn_event_list (GstAmlFacenet *filter, guint64 frameidx);

static gpointer facenet_input_process (void *data);
static gpointer facenet_result_process (void *data);
static gpointer facenet_db_process (void *data);
/* GObject vmethod implementations */

/* initialize the amlfacenet's class */
static void
gst_aml_facenet_class_init (GstAmlFacenetClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_aml_facenet_set_property;
  gobject_class->get_property = gst_aml_facenet_get_property;
  gobject_class->finalize = gst_aml_facenet_finalize;

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_DBPATH,
      g_param_spec_string ("db-path", "DB-Path",
        "Facenet db path", DEFAULT_PROP_DBPATH,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FORMAT,
      g_param_spec_string ("result-string-format", "Result-String-Format",
        "String format of detected result", DEFAULT_PROP_FORMAT,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_THRESHOLD,
      g_param_spec_float ("threshold", "Threshold",
        "Threshold of facenet recognition",
        0.1, 1.0, DEFAULT_PROP_THRESHOLD,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_STORE_FACE,
      g_param_spec_boolean ("store-face", "Store-Face",
          "Store the unrecongnized face (test only)",
          DEFAULT_PROP_STORE_FACE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


  gst_element_class_set_details_simple (gstelement_class,
      "amlfacenet",
      "Generic/Filter",
      "Amlogic FaceNet module",
      "Jemy Zhang <jun.zhang@amlogic.com>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_template));

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
    GST_DEBUG_FUNCPTR (gst_aml_facenet_transform_ip);

  GST_BASE_TRANSFORM_CLASS (klass)->set_caps =
    GST_DEBUG_FUNCPTR (gst_aml_facenet_set_caps);

  GST_BASE_TRANSFORM_CLASS (klass)->start =
    GST_DEBUG_FUNCPTR (gst_aml_facenet_open);
  GST_BASE_TRANSFORM_CLASS (klass)->stop =
    GST_DEBUG_FUNCPTR (gst_aml_facenet_close);

  GST_BASE_TRANSFORM_CLASS (klass)->sink_event =
    GST_DEBUG_FUNCPTR (gst_aml_facenet_event);
}

static void
thread_init (GstAmlFacenetThreadInfo *t) {
  g_cond_init (&t->cond);
  g_mutex_init (&t->mutex);
  t->thread = NULL;
}

static void
thread_release (GstAmlFacenetThreadInfo *t) {
  g_mutex_lock (&t->mutex);
  g_cond_signal (&t->cond);
  g_mutex_unlock (&t->mutex);
}

static void
thread_join (GstAmlFacenetThreadInfo *t) {
  if (t->thread) {
    g_thread_join (t->thread);
    t->thread = NULL;
  }
}

static void
open_facenet_db (GstAmlFacenet *filter) {
  if (filter->dbfile[0] == '\0') return;
  g_mutex_lock (&filter->procinfo_facenet_db.mutex);
  if (filter->db_handle) {
    db_deinit (filter->db_handle);
  }
  filter->db_handle = db_init (filter->dbfile);
  db_set_threshold (filter->threshold);
  g_mutex_unlock (&filter->procinfo_facenet_db.mutex);
}


/* initialize the new element
 * initialize instance structure
 */
static void
gst_aml_facenet_init (GstAmlFacenet *filter)
{
  filter->is_info_set = FALSE;
  filter->model_type = DEFAULT_PROP_MODEL_TYPE;
  filter->db_handle = NULL;
  filter->dbfile = g_strdup (DEFAULT_PROP_DBPATH);
  filter->string_format = g_strdup (DEFAULT_PROP_FORMAT);
  filter->b_store_face = DEFAULT_PROP_STORE_FACE;
  filter->threshold = DEFAULT_PROP_THRESHOLD;
  filter->is_facenet_proceeding = FALSE;
  filter->framenum = 0;

  list_init (&filter->face_list);
  list_init (&filter->facenet_ilist);
  list_init (&filter->facenet_rlist);


  thread_init (&filter->procinfo_facenet);
  thread_init (&filter->procinfo_facenet_result);
  thread_init (&filter->procinfo_facenet_db);
  thread_init (&filter->procinfo_grabface);

  list_init (&filter->nn_event_list);
  g_mutex_init (&filter->nn_event_list_mutex);

}

static void
gst_aml_facenet_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmlFacenet *filter = GST_AMLFACENET (object);

  switch (prop_id) {
    case PROP_DBPATH:
      {
        gchar *file = g_value_dup_string (value);
        if (g_strcmp0 (file, filter->dbfile)) {
          g_free(filter->dbfile);
          filter->dbfile = g_value_dup_string (value);
          open_facenet_db (filter);
        } else {
          g_free (file);
        }
      }
      break;
    case PROP_FORMAT:
      g_free(filter->string_format);
      filter->string_format = g_value_dup_string (value);
      break;
    case PROP_THRESHOLD:
      filter->threshold = g_value_get_float (value);
      db_set_threshold (filter->threshold);
      break;
    case PROP_STORE_FACE:
      filter->b_store_face = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_aml_facenet_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAmlFacenet *filter = GST_AMLFACENET (object);

  switch (prop_id) {
    case PROP_DBPATH:
      g_value_set_string (value, filter->dbfile);
      break;
    case PROP_FORMAT:
      g_value_set_string (value, filter->string_format);
      break;
    case PROP_THRESHOLD:
      g_value_set_float (value, filter->threshold);
      break;
    case PROP_STORE_FACE:
      g_value_set_boolean (value, filter->b_store_face);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_aml_facenet_open (GstBaseTransform * base)
{
  GstAmlFacenet *filter = GST_AMLFACENET (base);

  open_facenet_db (filter);

  det_set_log_config (DET_DEBUG_LEVEL_ERROR, DET_LOG_TERMINAL);

  if (filter->model_type == DET_BUTT) return FALSE;

  if (det_set_model (filter->model_type) != DET_STATUS_OK) {
    GST_ERROR_OBJECT (filter, "failed to initialize the nn detect library");
    return FALSE;
  }

  det_get_model_size (filter->model_type,
      &filter->model_width, &filter->model_height, &filter->model_channel);

  filter->_running = TRUE;
  filter->procinfo_facenet.thread = g_thread_new ("facenet set input", facenet_input_process, filter);
  filter->procinfo_facenet_result.thread = g_thread_new ("facenet wait result", facenet_result_process, filter);
  filter->procinfo_facenet_db.thread = g_thread_new ("facenet search db", facenet_db_process, filter);

  return TRUE;
}

static void
release_lists (GstAmlFacenet *filter) {
  struct listnode *pos, *q;
  if (!list_empty (&filter->face_list)) {
    list_for_each_safe (pos, q, &filter->face_list) {
      struct FaceInput *im =
        list_entry (pos, struct FaceInput, list);
      list_remove (pos);
      if (im->faceimg) g_free (im->faceimg);
      g_free (im);
    }
  }
  if (!list_empty (&filter->facenet_ilist)) {
    list_for_each_safe (pos, q, &filter->facenet_ilist) {
      struct FaceNetInput *im =
        list_entry (pos, struct FaceNetInput, list);
      list_remove (pos);
      if (im->faceimg) g_free (im->faceimg);
      g_free (im);
    }
  }
  if (!list_empty (&filter->facenet_rlist)) {
    list_for_each_safe (pos, q, &filter->facenet_ilist) {
      struct FaceNetResult *im =
        list_entry (pos, struct FaceNetResult, list);
      list_remove (pos);
      if (im->faceimg) g_free (im->faceimg);
      g_free (im);
    }
  }

  cleanup_nn_event_list (filter, 0);
}

static gboolean
gst_aml_facenet_close (GstBaseTransform * base)
{
  GstAmlFacenet *filter = GST_AMLFACENET (base);

  filter->_running = FALSE;

  thread_release (&filter->procinfo_facenet_db);
  thread_release (&filter->procinfo_facenet_result);
  thread_release (&filter->procinfo_facenet);
  thread_release (&filter->procinfo_grabface);

  thread_join (&filter->procinfo_facenet_db);
  thread_join (&filter->procinfo_facenet_result);
  thread_join (&filter->procinfo_facenet);
  thread_join (&filter->procinfo_grabface);

  if (det_release_model (filter->model_type) != DET_STATUS_OK) {
    return FALSE;
  }
  filter->model_type = DET_BUTT;

  db_deinit (filter->db_handle);
  filter->db_handle = NULL;
  frmcrop_deinit ();
  release_lists (filter);

  return TRUE;
}

#define FREE_STRING(s) \
  do { \
    if (s) { \
      g_free (s); \
      s = NULL; \
    } \
  } while(0)

static void
gst_aml_facenet_finalize (GObject * object)
{
  GstAmlFacenet *filter = GST_AMLFACENET (object);
  FREE_STRING (filter->dbfile);
  FREE_STRING (filter->string_format);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static gboolean
gst_aml_facenet_set_caps (GstBaseTransform * base, GstCaps * incaps, GstCaps * outcaps)
{
  GstAmlFacenet *filter = GST_AMLFACENET (base);
  GstVideoInfo info;

  if (!gst_video_info_from_caps (&info, incaps))
  {
    GST_ERROR_OBJECT (base, "caps are invalid");
    return FALSE;
  }
  filter->info = info;
  filter->is_info_set = TRUE;
  return TRUE;
}

static void
cleanup_nn_event_list (GstAmlFacenet *filter, guint64 frameidx) {
  struct listnode *pos, *q;
  if (!list_empty (&filter->nn_event_list)) {
    list_for_each_safe (pos, q, &filter->nn_event_list) {
      struct NNResult *im =
        list_entry (pos, struct NNResult, list);
      if (frameidx < im->frameidx ||
          frameidx - im->frameidx > 1) {
        list_remove (pos);
        if (im->pt) g_free (im->pt);
        g_free (im);
      }
    }
  }
}

/* GstBaseTransform vmethod implementations */
static gboolean
gst_aml_facenet_event (GstBaseTransform * base, GstEvent *event)
{
  GstAmlFacenet *filter = GST_AMLFACENET (base);
  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CUSTOM_DOWNSTREAM_OOB:
      {
        const GstStructure *resst = gst_event_get_structure (event);
        if (gst_structure_has_name (resst, "nn-detection")) {
          GstMapInfo info;
          const GValue *idx = gst_structure_get_value (resst, "idx");
          const GValue *size = gst_structure_get_value (resst, "rectnum");
          const GValue *buf = gst_structure_get_value (resst, "rectbuf");
          guint64 frameidx = g_value_get_uint64 (idx);
          gint detect_num = g_value_get_int (size);
          GstBuffer *resbuf = gst_value_get_buffer (buf);
          struct RelativePos *pt = (struct RelativePos *)g_malloc (sizeof(struct RelativePos) * detect_num);
          if (gst_buffer_map (resbuf, &info, GST_MAP_READ)) {
            g_memmove (pt, info.data, info.size);
            gst_buffer_unmap (resbuf, &info);
          } else {
            g_free (pt);
            pt = NULL;
          }

          if (pt) {
            struct NNResult *new_result = (struct NNResult *)g_malloc (sizeof(struct NNResult));
            new_result->detect_num = detect_num;
            new_result->frameidx = frameidx;
            new_result->pt = pt;
            list_init (&new_result->list);
            g_mutex_lock (&filter->nn_event_list_mutex);
            // add newest entries
            list_add_tail (&filter->nn_event_list, &new_result->list);
            g_mutex_unlock (&filter->nn_event_list_mutex);
          }

          //gst_event_unref (event);
          //return FALSE;
        }
      }
      break;
    default:
      break;
  }
  return GST_BASE_TRANSFORM_CLASS(parent_class)->sink_event (base, event);

}

static void
push_result (GstBaseTransform *base, struct FaceNetDBResult *result)
{
  GstMapInfo info;

  int res_size = sizeof (result->pos);
  int info_size = result->info ? strlen(result->info) + 1 : 0;

  GstBuffer *resbuf = gst_buffer_new_allocate (NULL, res_size + info_size, NULL);
  if (gst_buffer_map (resbuf, &info, GST_MAP_WRITE)) {
    g_memmove (info.data, &result->pos, res_size);
    g_memmove (info.data + res_size, result->info, info_size);
    gst_buffer_unmap (resbuf, &info);

    GstStructure *resst = gst_structure_new ("facenet-detection",
        "idx", G_TYPE_UINT64, result->frameidx,
        "faceinfo", GST_TYPE_BUFFER, resbuf,
        NULL);

    GstEvent *facenet_detect_event = gst_event_new_custom (GST_EVENT_CUSTOM_DOWNSTREAM_OOB,
        resst);

    gst_element_send_event (&base->element, facenet_detect_event);
  }

  gst_buffer_unref (resbuf);

}

static gpointer
facenet_db_process (void *data) {
  GstAmlFacenet *filter = (GstAmlFacenet *)data;
  // set database

  while (filter->_running) {
    g_mutex_lock (&filter->procinfo_facenet_result.mutex);
    if (list_empty (&filter->facenet_rlist)) {
      g_cond_wait (&filter->procinfo_facenet_result.cond, &filter->procinfo_facenet_result.mutex);
    }

    if (list_empty (&filter->facenet_rlist)) {
      g_mutex_unlock (&filter->procinfo_facenet_result.mutex);
      continue;
    }

    struct listnode *node = list_head (&filter->facenet_rlist);
    list_remove (node);
    g_mutex_unlock (&filter->procinfo_facenet_result.mutex);

    struct FaceNetResult *item =
      list_entry (node, struct FaceNetResult, list);

#define FACE_INFO_BUFSIZE 1024
    char *buf = (char *) g_malloc (FACE_INFO_BUFSIZE);
    struct FaceNetDBResult result;

    result.frameidx = item->frameidx;
    result.pos = item->pos;
    result.info = NULL;
    g_mutex_lock (&filter->procinfo_facenet_db.mutex);
    GST_INFO_OBJECT (filter, "looking up db");
    int rc = db_search_result(filter->db_handle, item->result,
          filter->b_store_face ? item->faceimg : NULL,
          filter->model_width, filter->model_height,
          filter->string_format, buf, FACE_INFO_BUFSIZE);
    GST_INFO_OBJECT (filter, "looking up db done");
    g_mutex_unlock (&filter->procinfo_facenet_db.mutex);

    if (rc == 0) {
      result.info = buf;
    }

    push_result (&filter->element, &result);

    g_free (buf);
    if (item->faceimg) g_free (item->faceimg);
    g_free (item);
  }

  return NULL;

}

static gpointer
facenet_result_process (void *data) {
  DetectResult result;
  GstAmlFacenet *filter = (GstAmlFacenet *)data;

  while (filter->_running) {
    g_mutex_lock (&filter->procinfo_facenet.mutex);
    if (list_empty (&filter->facenet_ilist)) {
      g_cond_wait (&filter->procinfo_facenet.cond, &filter->procinfo_facenet.mutex);
    }

    if (list_empty (&filter->facenet_ilist)) {
      g_mutex_unlock (&filter->procinfo_facenet.mutex);
      continue;
    }

    struct listnode *node = list_head (&filter->facenet_ilist);
    list_remove (node);

    struct FaceNetInput *item =
      list_entry (node, struct FaceNetInput, list);

    GST_INFO_OBJECT (filter, "waiting for result");
    det_status_t rc = det_get_result(&result, filter->model_type);
    GST_INFO_OBJECT (filter, "result got");
    filter->is_facenet_proceeding = FALSE;
    g_mutex_unlock (&filter->procinfo_facenet.mutex);

    if (rc == DET_STATUS_OK) {
      g_mutex_lock (&filter->procinfo_facenet_result.mutex);
      // add result to list
      struct FaceNetResult *newitem =
        (struct FaceNetResult*) g_malloc (sizeof (struct FaceNetResult));

      newitem->frameidx = item->frameidx;
      newitem->pos = item->pos;
      newitem->faceimg = item->faceimg;
      g_memmove (newitem->result, result.facenet_result, sizeof(newitem->result));

      list_init (&newitem->list);
      list_add_tail (&filter->facenet_rlist, &newitem->list);

      g_cond_signal (&filter->procinfo_facenet_result.cond);
      g_mutex_unlock (&filter->procinfo_facenet_result.mutex);
    }
    g_free (item);

  }

  return NULL;
}

static gpointer
facenet_input_process (void *data) {
  GstAmlFacenet *filter = (GstAmlFacenet *)data;

  while (filter->_running) {
    g_mutex_lock (&filter->procinfo_grabface.mutex);

    if (list_empty (&filter->face_list)) {
      g_cond_wait (&filter->procinfo_grabface.cond, &filter->procinfo_grabface.mutex);
    }

    if (list_empty (&filter->face_list) ||
        filter->is_facenet_proceeding) {
      g_mutex_unlock (&filter->procinfo_grabface.mutex);
      continue;
    }

    struct listnode *node = list_head (&filter->face_list);
    list_remove (node);
    g_mutex_unlock (&filter->procinfo_grabface.mutex);

    struct FaceInput *item =
      list_entry (node, struct FaceInput, list);
    input_image_t im;
    im.data = item->faceimg;
    im.pixel_format = PIX_FMT_RGB888;
    im.width = filter->model_width;
    im.height = filter->model_height;
    im.channel = filter->model_channel;

    if (g_mutex_trylock (&filter->procinfo_facenet.mutex)) {
      if (det_set_input (im, filter->model_type) == DET_STATUS_OK) {
        filter->is_facenet_proceeding = TRUE;

        struct FaceNetInput *newitem =
          (struct FaceNetInput*) g_malloc (sizeof (struct FaceNetInput));

        newitem->frameidx = item->frameidx;
        newitem->pos = item->pos;
        newitem->faceimg = item->faceimg;

        list_init (&newitem->list);
        list_add_tail (&filter->facenet_ilist, &newitem->list);

        g_cond_signal (&filter->procinfo_facenet.cond);
        g_mutex_unlock (&filter->procinfo_facenet.mutex);
      }
    }

    g_free (item);
  }

  return NULL;

}

/* this function does the actual processing
*/
static GstFlowReturn
gst_aml_facenet_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstAmlFacenet *filter = GST_AMLFACENET (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (GST_OBJECT (filter), GST_BUFFER_TIMESTAMP (outbuf));

  if (!filter->is_info_set) {
    GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL), ("unknown format"));
    return GST_FLOW_NOT_NEGOTIATED;
  }

  filter->framenum ++;

  g_mutex_lock (&filter->nn_event_list_mutex);
  // cleanup the old entries
  cleanup_nn_event_list (filter, filter->framenum);

  if (list_empty (&filter->nn_event_list)) {
    g_mutex_unlock (&filter->nn_event_list_mutex);
    return GST_FLOW_OK;
  }

  struct listnode *node = list_head (&filter->nn_event_list);
  list_remove (node);
  g_mutex_unlock (&filter->nn_event_list_mutex);

  struct NNResult *item =
    list_entry (node, struct NNResult, list);

  if (item->detect_num > 0) {
    GstVideoInfo *info = &filter->info;
    GstMapInfo outbuf_info;
    if (gst_buffer_map (outbuf, &outbuf_info, GST_MAP_READ)) {
      for (int i = 0; i < item->detect_num; i++) {
        struct RelativePos *pos = &item->pt[i];
        struct FaceInput *newitem =
          (struct FaceInput *) g_malloc (sizeof(struct FaceInput));
        newitem->frameidx = filter->framenum;
        newitem->pos = *pos;

        int x0, y0, x1, y1;
        x0 = (int)(pos->x0 * info->width);
        y0 = (int)(pos->y0 * info->height);
        x1 = (int)(pos->x1 * info->width);
        y1 = (int)(pos->y1 * info->height);

        frmcrop_init ();

        newitem->faceimg = frmcrop_begin (outbuf_info.data,
            info->width, info->height, x0, y0, x1, y1,
            filter->model_width, filter->model_height);
        list_init (&newitem->list);

        if (g_mutex_trylock (&filter->procinfo_grabface.mutex)) {
          list_add_tail (&filter->face_list, &newitem->list);
          g_cond_signal (&filter->procinfo_grabface.cond);
          g_mutex_unlock (&filter->procinfo_grabface.mutex);
        } else {
          g_free (newitem->faceimg);
          g_free (newitem);
        }

      }
      frmcrop_end ();
      gst_buffer_unmap (outbuf, &outbuf_info);
    }
  }

  if (item->pt) g_free (item->pt);
  g_free (item);

  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
amlfacenet_init (GstPlugin * amlfacenet)
{
  GST_DEBUG_CATEGORY_INIT (gst_aml_facenet_debug, "amlfacenet", 0,
      "amlogic facenet element");

  return gst_element_register (amlfacenet, "amlfacenet", GST_RANK_PRIMARY,
      GST_TYPE_AMLFACENET);
}

/* gstreamer looks for this structure to register amlfacenets
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amlfacenet,
    "amlogic facenet plugins",
    amlfacenet_init,
    VERSION,
    "LGPL",
    "facenet plugins",
    "http://openlinux.amlogic.com"
    )
