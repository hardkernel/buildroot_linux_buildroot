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
 * SECTION:element-amlyoloface
 *
 * FIXME:Describe amlyoloface here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amlyoloface ! fakesink silent=TRUE
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

#include "gstamlyoloface.h"

GST_DEBUG_CATEGORY_STATIC (gst_aml_yolo_face_debug);
#define GST_CAT_DEFAULT gst_aml_yolo_face_debug

#define HAVE_AMLYOLOFACE_ADDITIONAL_LIBRARIES "/usr/lib/libyoloface.so"


struct _GstAmlYoloFaceVTable
{
  GModule *module;

  int (*yoloface_init) (const char *model_path);
  int (*yoloface_process) (const char *buf, int width, int height);
  void (*yoloface_deinit) ();
  DetectResult * (*yoloface_get_detection_result) ();
};

static GstAmlYoloFaceVTable *vtable_aml = NULL;

#define LOAD_SYMBOL(name) G_STMT_START { \
  if (!g_module_symbol (module, #name, (gpointer *) &vtable->name)) { \
    GST_ERROR ("Failed to load '" #name "' from '%s'", filename); \
    goto error; \
  } \
} G_STMT_END;

static GstAmlYoloFaceVTable *
load_yoloface (const gchar * filename)
{
  GModule *module;
  GstAmlYoloFaceVTable *vtable;

  module = g_module_open (filename, G_MODULE_BIND_LOCAL);
  if (!module) {
    GST_ERROR ("Failed to load '%s'", filename);
    return NULL;
  }

  vtable = g_new0 (GstAmlYoloFaceVTable, 1);
  vtable->module = module;

  if (!g_module_symbol (module, G_STRINGIFY (yoloface_init),
          (gpointer *) & vtable->yoloface_init)) {
    GST_ERROR ("Failed to load '" G_STRINGIFY (yoloface_init)
        "' from '%s'. Incompatible version?", filename);
    goto error;
  }

  LOAD_SYMBOL (yoloface_init);
  LOAD_SYMBOL (yoloface_process);
  LOAD_SYMBOL (yoloface_get_detection_result);
  LOAD_SYMBOL (yoloface_deinit);

  return vtable;

error:
  g_module_close (vtable->module);
  g_free (vtable);
  return NULL;
}

static void
unload_yoloface (GstAmlYoloFaceVTable * vtable)
{
  if (vtable->module) {
    g_module_close (vtable->module);
    g_free (vtable);
  }
}

#undef LOAD_SYMBOL

static gboolean
load_libraries (void)
{
  gchar **libraries = g_strsplit (HAVE_AMLYOLOFACE_ADDITIONAL_LIBRARIES, ":", -1);
  gchar **p = libraries;

  while (*p && !vtable_aml) {
    GstAmlYoloFaceVTable *vtable = load_yoloface (*p);

    if (vtable) {
      if (!vtable_aml) {
        vtable_aml = vtable;
      } else {
        unload_yoloface (vtable);
      }
    }

    p++;
  }
  g_strfreev (libraries);

  if (!vtable_aml)
    return FALSE;

  return TRUE;
}
/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

#define PROP_DEFAULT_DETECT_INTERVAL 3
#define PROP_MAX_DETECT_INTERVAL 10

enum
{
  PROP_0,
  PROP_DETECT_INTERVAL,
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

#define gst_aml_yolo_face_parent_class parent_class
G_DEFINE_TYPE (GstAmlYoloFace, gst_aml_yolo_face, GST_TYPE_BASE_TRANSFORM);

static void gst_aml_yolo_face_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_aml_yolo_face_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

typedef enum LINE_DIRECTION_e {
  LINE_DIRECTION_VERTICAL,
  LINE_DIRECTION_HORIZONTAL,
} LINE_DIRECTION_t;

static void
draw_line (gchar *buf, gint buf_w, gint buf_h, gint x0, gint y0, gint length, gint thickness, LINE_DIRECTION_t dir) {
  gint pos;
  gchar *pfill;

  if (length <=0) return;

  thickness =  thickness <= 0 ? 1 : thickness;

  if (dir == LINE_DIRECTION_HORIZONTAL) {
    if (y0 + thickness > buf_h) {
      y0 = buf_h - thickness;
    }
    length += thickness;
    if (x0 + length > buf_w) {
      length = buf_w - x0;
    }
    for (gint t = 0; t < thickness; t++) {
      pos = x0 + (y0 + t) * buf_w;
      pfill = buf + pos * 3;
      for (gint w = 0; w < length; w++) {
        *pfill++ = 255;
        *pfill++ = 0;
        *pfill++ = 0;
      }
    }
  } else {
    if (x0 + thickness > buf_w) {
      x0 = buf_w - thickness;
    }
    for (gint h = 0; h < length; h++) {
      pos = x0 + (y0 + h) * buf_w;
      pfill = buf + pos * 3;
      for (gint t = 0; t < thickness; t++) {
        *pfill++ = 255;
        *pfill++ = 0;
        *pfill++ = 0;
      }
    }
  }

}

static void
draw_rect (gchar *buf, gint w, gint h, gint x0, gint y0, gint x1, gint y1, gint thickness) {
  if (x1 < x0) {
    gint temp = x1;
    x1 = x0;
    x0 = temp;
  }
  if (y1 < y0) {
    gint temp = y1;
    y1 = y0;
    y0 = temp;
  }

  if (x1 >= w || y1 >= h) {
    return;
  }

  gint fill_w = x1 - x0;
  gint fill_h = y1 - y0;
  thickness = thickness > fill_w ? fill_w : thickness;
  thickness = thickness > fill_h ? fill_h : thickness;

  draw_line (buf, w, h, x0, y0, fill_w, thickness, LINE_DIRECTION_HORIZONTAL);
  draw_line (buf, w, h, x0, y1, fill_w, thickness, LINE_DIRECTION_HORIZONTAL);
  draw_line (buf, w, h, x0, y0, fill_h, thickness, LINE_DIRECTION_VERTICAL);
  draw_line (buf, w, h, x1, y0, fill_h, thickness, LINE_DIRECTION_VERTICAL);

}

static gboolean gst_aml_yolo_face_open (GstAmlYoloFace * base);

static gboolean gst_aml_yolo_face_close (GstAmlYoloFace * base);

static void gst_aml_yolo_face_finalize (GObject * object);

static GstFlowReturn gst_aml_yolo_face_transform_ip (GstBaseTransform * base,
    GstBuffer * outbuf);

static gboolean gst_aml_yolo_face_set_caps (GstBaseTransform * base,
    GstCaps * in, GstCaps * out);
/* GObject vmethod implementations */

/* initialize the amlyoloface's class */
static void
gst_aml_yolo_face_class_init (GstAmlYoloFaceClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_aml_yolo_face_set_property;
  gobject_class->get_property = gst_aml_yolo_face_get_property;
  gobject_class->finalize = gst_aml_yolo_face_finalize;

  g_object_class_install_property (gobject_class, PROP_DETECT_INTERVAL,
      g_param_spec_int ("interval", "Interval", "Detection intervals(every n frames). 0: detect every frame",
          0, PROP_MAX_DETECT_INTERVAL, PROP_DEFAULT_DETECT_INTERVAL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_details_simple (gstelement_class,
    "AmlYoloFace",
    "Generic/Filter",
    "Face Detection module for amlogic",
    "Jemy Zhang <jun.zhang@amlogic.com>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_aml_yolo_face_transform_ip);

  GST_BASE_TRANSFORM_CLASS (klass)->set_caps =
      GST_DEBUG_FUNCPTR (gst_aml_yolo_face_set_caps);

}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_aml_yolo_face_init (GstAmlYoloFace *filter)
{
  filter->vtable = NULL;
  filter->is_facelib_inited = FALSE;
  filter->is_info_set = FALSE;
  filter->detect_interval = PROP_DEFAULT_DETECT_INTERVAL;
  filter->frame_cnt = 0;
  filter->result = NULL;
  gst_aml_yolo_face_open (filter);
}

static void
gst_aml_yolo_face_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmlYoloFace *filter = GST_AMLYOLOFACE (object);

  switch (prop_id) {
    case PROP_DETECT_INTERVAL:
      filter->detect_interval = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_aml_yolo_face_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAmlYoloFace *filter = GST_AMLYOLOFACE (object);

  switch (prop_id) {
    case PROP_DETECT_INTERVAL:
      g_value_set_int (value, filter->detect_interval);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_aml_yolo_face_open (GstAmlYoloFace * filter)
{
  if (filter->vtable == NULL) {
    filter->vtable = vtable_aml;
    g_assert (filter->vtable != NULL);
  }
  if (!filter->is_facelib_inited) {
    if (filter->vtable->yoloface_init (NULL) == 0) {
      GST_ERROR_OBJECT (filter, "failed to initialize the yoloface library");
      return FALSE;
    }
    filter->is_facelib_inited = TRUE;
  }
  return TRUE;
}

static gboolean
gst_aml_yolo_face_close (GstAmlYoloFace * filter)
{
  if (filter->is_facelib_inited) {
    filter->vtable->yoloface_deinit ();
    filter->is_facelib_inited = FALSE;
  }
  return TRUE;
}

static void
gst_aml_yolo_face_finalize (GObject * object)
{
  GstAmlYoloFace *filter = GST_AMLYOLOFACE (object);

  gst_aml_yolo_face_close (filter);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static gboolean
gst_aml_yolo_face_set_caps (GstBaseTransform * base, GstCaps * in, GstCaps * out)
{
  GstAmlYoloFace *filter = GST_AMLYOLOFACE (base);
  GstVideoInfo in_info;

  if (!gst_video_info_from_caps (&in_info, in))
  {
    GST_ERROR_OBJECT (base, "caps are invalid");
    return FALSE;
  }
  filter->info = in_info;
  filter->is_info_set = TRUE;
  return TRUE;
}

/* GstBaseTransform vmethod implementations */

/* this function does the actual processing
 */
static GstFlowReturn
gst_aml_yolo_face_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstAmlYoloFace *filter = GST_AMLYOLOFACE (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (GST_OBJECT (filter), GST_BUFFER_TIMESTAMP (outbuf));

  if (!filter->is_info_set) {
    GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL), ("unknown format"));
    return GST_FLOW_NOT_NEGOTIATED;
  }

  if (filter->vtable == NULL) {
    GST_ERROR_OBJECT (base, "yoloface library not loaded");
    return GST_FLOW_ERROR;
  }

  if (!filter->is_facelib_inited) {
    GST_ERROR_OBJECT (base, "yoloface library not intialized");
    return GST_FLOW_ERROR;
  }

  filter->frame_cnt ++;

  GstVideoInfo *info = &filter->info;
  GstMapInfo outbuf_info;
  if (gst_buffer_map (outbuf, &outbuf_info, GST_MAP_READ)) {
    if (filter->detect_interval == 0 ||
        (filter->frame_cnt % (filter->detect_interval + 1)) == 0) {
      if (filter->vtable->yoloface_process (outbuf_info.data, info->width, info->height) != 0) {
        filter->result = filter->vtable->yoloface_get_detection_result ();
      } else {
        filter->result = NULL;
      }
    }
    if (filter->result) {
      for (int i = 0; i < filter->result->detect_num; i++) {
        draw_rect (outbuf_info.data, info->width, info->height,
            filter->result->pt[i].left, filter->result->pt[i].top,
            filter->result->pt[i].right, filter->result->pt[i].bottom, 2);
      }
    }
    gst_buffer_unmap (outbuf, &outbuf_info);
  }

  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
amlyoloface_init (GstPlugin * amlyoloface)
{
  GST_DEBUG_CATEGORY_INIT (gst_aml_yolo_face_debug, "amlyoloface", 0,
      "amlogic face detection element");

  if (!load_libraries ())
    return FALSE;
  return gst_element_register (amlyoloface, "amlyoloface", GST_RANK_PRIMARY,
      GST_TYPE_AMLYOLOFACE);
}

/* gstreamer looks for this structure to register amlyolofaces
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amlyoloface,
    "face detection plugins for amlogic",
    amlyoloface_init,
    VERSION,
    "LGPL",
    "face detection plugins",
    "http://openlinux.amlogic.com"
)
