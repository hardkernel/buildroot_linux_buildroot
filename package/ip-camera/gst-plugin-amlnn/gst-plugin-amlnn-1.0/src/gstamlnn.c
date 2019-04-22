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
 * SECTION:element-amlnn
 *
 * FIXME:Describe amlnn here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amlnn ! fakesink silent=TRUE
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

#include "frameresize.h"

#include "gstamlnn.h"

GST_DEBUG_CATEGORY_STATIC (gst_aml_nn_debug);
#define GST_CAT_DEFAULT gst_aml_nn_debug

typedef struct Relative_DetectPoint {
  float rel_left;
  float rel_top;
  float rel_right;
  float rel_bottom;
} RDetectPoint_t;

#define DEFAULT_PROP_MODEL_TYPE DET_YOLOFACE_V2

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_MODEL_TYPE,
};

#define GST_TYPE_AML_DET_MODEL_TYPE (gst_aml_detect_model_get_type())
static GType
gst_aml_detect_model_get_type (void)
{
  static GType aml_detect_model_type = 0;
  static const GEnumValue aml_detect_model [] = {
    {DET_YOLOFACE_V2, "yoloface-v2", "yoloface v2"},
    {DET_YOLO_V2, "yolo-v2", "yolo v2"},
    {DET_YOLO_V3, "yolo-v3", "yolo v3"},
    {DET_YOLO_TINY, "yolo-tiny", "yolo tiny"},
    {DET_SSD, "ssd", "ssd"},
    {DET_MTCNN_V1, "mtcnn-v1", "mtcnn v1"},
	{DET_MTCNN_V2, "mtcnn-v2", "mtcnn v2"},
	{DET_FASTER_RCNN, "faster-rcnn", "faster rcnn"},
	{DET_DEEPLAB_V1, "deeplab-v1", "deeplab v1"},
	{DET_DEEPLAB_V2, "deeplab-v2", "deeplab v2"},
	{DET_DEEPLAB_V3, "deeplab-v3", "deeplab v3"},
    {0, NULL, NULL},
  };

  if (!aml_detect_model_type) {
    aml_detect_model_type =
        g_enum_register_static ("GstAMLDetectModel",
        aml_detect_model);
  }
  return aml_detect_model_type;
}

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


#define gst_aml_nn_parent_class parent_class
G_DEFINE_TYPE (GstAmlNN, gst_aml_nn, GST_TYPE_BASE_TRANSFORM);

static void gst_aml_nn_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_aml_nn_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_aml_nn_open (GstBaseTransform * base);

static gboolean gst_aml_nn_close (GstBaseTransform * base);

static void gst_aml_nn_finalize (GObject * object);

static GstFlowReturn gst_aml_nn_transform_ip (GstBaseTransform * base,
    GstBuffer * outbuf);

static gboolean gst_aml_nn_set_caps (GstBaseTransform * base,
    GstCaps * incaps, GstCaps * outcaps);

static gpointer detect_result_process (void *data);
/* GObject vmethod implementations */

/* initialize the amlnn's class */
static void
gst_aml_nn_class_init (GstAmlNNClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_aml_nn_set_property;
  gobject_class->get_property = gst_aml_nn_get_property;
  gobject_class->finalize = gst_aml_nn_finalize;

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_MODEL_TYPE,
      g_param_spec_enum ("model-type", "model-type",
          "detection model type", GST_TYPE_AML_DET_MODEL_TYPE,
          DEFAULT_PROP_MODEL_TYPE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_details_simple (gstelement_class,
    "amlnn",
    "Generic/Filter",
    "Amlogic NN Detection module",
    "Jemy Zhang <jun.zhang@amlogic.com>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_template));

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_aml_nn_transform_ip);

  GST_BASE_TRANSFORM_CLASS (klass)->set_caps =
      GST_DEBUG_FUNCPTR (gst_aml_nn_set_caps);

  GST_BASE_TRANSFORM_CLASS (klass)->start =
      GST_DEBUG_FUNCPTR (gst_aml_nn_open);
  GST_BASE_TRANSFORM_CLASS (klass)->stop =
      GST_DEBUG_FUNCPTR (gst_aml_nn_close);
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_aml_nn_init (GstAmlNN *filter)
{
  filter->is_info_set = FALSE;
  filter->model_type = DEFAULT_PROP_MODEL_TYPE;
  filter->b_model_set = FALSE;

  g_cond_init (&filter->_cond);
  g_mutex_init (&filter->_mutex);
}

static void
change_model (GstAmlNN *filter, det_model_type type) {
  GST_ERROR_OBJECT (filter, "set model type = %d", type);
  if (type != filter->model_type) {
    if (filter->b_model_set) {
      g_mutex_lock (&filter->_mutex);
      det_release_model (filter->model_type);
      det_set_model (type);
      det_get_model_size (type,
          &filter->model_width, &filter->model_height, &filter->model_channel);
      g_mutex_unlock (&filter->_mutex);
    }
    filter->model_type = type;
  }

}

static void
gst_aml_nn_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmlNN *filter = GST_AMLNN (object);

  switch (prop_id) {
    case PROP_MODEL_TYPE:
      change_model (filter, g_value_get_enum (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_aml_nn_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAmlNN *filter = GST_AMLNN (object);

  switch (prop_id) {
    case PROP_MODEL_TYPE:
      g_value_set_enum (value, filter->model_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_aml_nn_open (GstBaseTransform* sink)
{
  GstAmlNN *filter = GST_AMLNN (sink);

  filter->framerate = 30;
  filter->framenum = 0;
  filter->next_detect_framenum = 0;

  if (filter->b_model_set) return TRUE;


  det_set_log_config (DET_DEBUG_LEVEL_ERROR, DET_LOG_TERMINAL);

  if (filter->model_type == DET_BUTT) return FALSE;

  if (det_set_model (filter->model_type) != DET_STATUS_OK) {
    GST_ERROR_OBJECT (filter, "failed to initialize the nn detect library");
    return FALSE;
  }

  det_get_model_size (filter->model_type,
      &filter->model_width, &filter->model_height, &filter->model_channel);
  filter->b_model_set = TRUE;
  filter->_running = TRUE;
  filter->_thread = g_thread_new ("detect process", detect_result_process, filter);
  return TRUE;
}

static gboolean
gst_aml_nn_close (GstBaseTransform* sink)
{
  GstAmlNN *filter = GST_AMLNN (sink);

  if (!filter->b_model_set) return TRUE;

  GST_INFO_OBJECT (filter, "closing, waiting for lock");
  filter->_running = FALSE;
  g_mutex_lock (&filter->_mutex);
  if (det_release_model (filter->model_type) != DET_STATUS_OK) {
    GST_ERROR_OBJECT (filter, "failed to release nn detect model");
  }
  GST_INFO_OBJECT (filter, "model released");
  filter->model_type = DET_BUTT;
  filter->b_model_set = FALSE;
  g_cond_signal (&filter->_cond);
  g_mutex_unlock (&filter->_mutex);
  g_thread_join (filter->_thread);

  filter->_thread = NULL;

  frameresize_deinit ();
  filter->is_info_set = FALSE;

  return TRUE;
}

static void
gst_aml_nn_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static gboolean
gst_aml_nn_set_caps (GstBaseTransform * base, GstCaps * incaps, GstCaps * outcaps)
{
  GstAmlNN *filter = GST_AMLNN (base);
  GstVideoInfo info;

  if (!gst_video_info_from_caps (&info, incaps))
  {
    GST_ERROR_OBJECT (base, "caps are invalid");
    return FALSE;
  }
  filter->info = info;
  filter->is_info_set = TRUE;
  filter->framerate = filter->info.fps_d == 0 ? 0 :
    filter->info.fps_n / filter->info.fps_d;
  return TRUE;
}

/* GstBaseTransform vmethod implementations */
static void
push_result (GstBaseTransform * base, DetectResult *result, guint64 framenum)
{
  GstMapInfo info;
  int i;

  if (result->detect_num <= 0) return;
  if (result->point[0].type != DET_RECTANGLE_TYPE) return;

  int res_size = result->detect_num * sizeof (RDetectPoint_t);
  RDetectPoint_t *pres = (RDetectPoint_t *)g_malloc (res_size);
  if (pres == NULL) return;

  for (i = 0; i < result->detect_num; i++) {
    pres[i].rel_left = result->point[i].point.rectPoint.left;
    pres[i].rel_top = result->point[i].point.rectPoint.top;
    pres[i].rel_right = result->point[i].point.rectPoint.right;
    pres[i].rel_bottom = result->point[i].point.rectPoint.bottom;
  }

  GstBuffer *resbuf = gst_buffer_new_allocate (NULL, res_size, NULL);
  if (gst_buffer_map (resbuf, &info, GST_MAP_WRITE)) {
    g_memmove (info.data, pres, res_size);
    gst_buffer_unmap (resbuf, &info);
    GstStructure *resst = gst_structure_new ("nn-detection",
        "idx", G_TYPE_UINT64, framenum,
        "rectnum", G_TYPE_INT, result->detect_num,
        "rectbuf", GST_TYPE_BUFFER, resbuf,
        NULL);

    GstEvent *nn_detect_event = gst_event_new_custom (GST_EVENT_CUSTOM_DOWNSTREAM_OOB,
        resst);

    gst_element_send_event (&base->element, nn_detect_event);
  }
  gst_buffer_unref (resbuf);
  g_free (pres);
}

static gpointer
detect_result_process (void *data) {
  DetectResult result;
  GstAmlNN *filter = (GstAmlNN *)data;

  while (filter->_running) {
    g_mutex_lock (&filter->_mutex);
    g_cond_wait (&filter->_cond, &filter->_mutex);

    if (filter->model_type == DET_BUTT
        || filter->b_model_set == FALSE) {
      g_mutex_unlock (&filter->_mutex);
      continue;
    }

    GST_INFO_OBJECT (filter, "waiting for result");
    det_status_t rc = det_get_result(&result, filter->model_type);
    filter->next_detect_framenum = filter->framenum +
      (result.detect_num > 8 ? 8 : result.detect_num) * 20 * filter->framerate / 1000;
    GST_INFO_OBJECT (filter, "result got, set next expect framenum: %lu", filter->next_detect_framenum);
    g_mutex_unlock (&filter->_mutex);

    if (rc == DET_STATUS_OK) {
      // check running status to avoid async problem after long time delay
      if (filter->_running) {
        push_result (&filter->element, &result, filter->framenum);
      } else {
        GST_INFO_OBJECT (filter, "thread exiting, push event abort");
      }
    }

  }

  return NULL;

}

/* this function does the actual processing
 */
static GstFlowReturn
gst_aml_nn_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstAmlNN *filter = GST_AMLNN (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (GST_OBJECT (filter), GST_BUFFER_TIMESTAMP (outbuf));


  if (!filter->is_info_set) {
    GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL), ("unknown format"));
    return GST_FLOW_NOT_NEGOTIATED;
  }

  filter->framenum ++;

  if (filter->next_detect_framenum > 0 &&
      filter->framenum < filter->next_detect_framenum) {
    GST_INFO_OBJECT (base, "skip waiting for next frame, current frame %lu, expected %lu",
        filter->framenum, filter->next_detect_framenum);
    return GST_FLOW_OK;
  }

  GstVideoInfo *info = &filter->info;
  GstMapInfo outbuf_info;

  if (g_mutex_trylock (&filter->_mutex)) {
    if (gst_buffer_map (outbuf, &outbuf_info, GST_MAP_READ)) {
      frameresize_init ();
      char *frm = frameresize_begin ((char *)outbuf_info.data,
          info->width, info->height, filter->model_width, filter->model_height);
      if (frm) {
        input_image_t im;
        im.data = (unsigned char*)frm;
        im.pixel_format = PIX_FMT_RGB888;
        im.width = filter->model_width;
        im.height = filter->model_height;
        im.channel = filter->model_channel;
        if (det_set_input (im, filter->model_type) == DET_STATUS_OK) {
          g_cond_signal (&filter->_cond);
        }
        frameresize_end (frm);
      }
      gst_buffer_unmap (outbuf, &outbuf_info);
    }
    g_mutex_unlock (&filter->_mutex);
  }

  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
amlnn_init (GstPlugin * amlnn)
{
  GST_DEBUG_CATEGORY_INIT (gst_aml_nn_debug, "amlnn", 0,
      "amlogic nn detection element");

  return gst_element_register (amlnn, "amlnn", GST_RANK_PRIMARY,
      GST_TYPE_AMLNN);
}

/* gstreamer looks for this structure to register amlnns
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amlnn,
    "amlogic nn detection plugins",
    amlnn_init,
    VERSION,
    "LGPL",
    "nn detection plugins",
    "http://openlinux.amlogic.com"
)
