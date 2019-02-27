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
 * SECTION:element-amlimgcap
 *
 * FIXME:Describe amlimgcap here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amlimgcap ! fakesink silent=TRUE
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

#include "gstamlimgcap.h"
#include "jpeg.h"

GST_DEBUG_CATEGORY_STATIC (gst_aml_imgcap_debug);
#define GST_CAT_DEFAULT gst_aml_imgcap_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

#define DEFAULT_PROP_QUALITY 80

enum
{
  PROP_0,
  PROP_QUALITY,
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

#define gst_aml_imgcap_parent_class parent_class
G_DEFINE_TYPE (GstAmlImageCap, gst_aml_imgcap, GST_TYPE_BASE_TRANSFORM);

static void gst_aml_imgcap_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_aml_imgcap_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_aml_imgcap_transform_ip (GstBaseTransform * base,
    GstBuffer * outbuf);

static gboolean gst_aml_imgcap_set_caps (GstBaseTransform * base,
    GstCaps * in, GstCaps * out);

static gboolean gst_aml_imgcap_sink_event (GstBaseTransform * base, GstEvent *event);
/* GObject vmethod implementations */

/* initialize the amlimgcap's class */
static void
gst_aml_imgcap_class_init (GstAmlImageCapClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseTransformClass *gstbasetransform_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasetransform_class = (GstBaseTransformClass *)klass;


  gobject_class->set_property = gst_aml_imgcap_set_property;
  gobject_class->get_property = gst_aml_imgcap_get_property;
  gstbasetransform_class->sink_event = gst_aml_imgcap_sink_event;

  g_object_class_install_property (gobject_class, PROP_QUALITY,
      g_param_spec_int ("quality", "Quality",
          "image quality", 1, 100,
          DEFAULT_PROP_QUALITY,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_details_simple (gstelement_class,
    "AmlOverlay",
    "Generic/Filter",
    "Face Detection module for amlogic",
    "Jemy Zhang <jun.zhang@amlogic.com>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_aml_imgcap_transform_ip);

  GST_BASE_TRANSFORM_CLASS (klass)->set_caps =
      GST_DEBUG_FUNCPTR (gst_aml_imgcap_set_caps);

}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_aml_imgcap_init (GstAmlImageCap *imgcap)
{
  imgcap->is_info_set = FALSE;
  imgcap->quality = DEFAULT_PROP_QUALITY;
  imgcap->location = NULL;
  imgcap->b_capreq = FALSE;
}

static void
gst_aml_imgcap_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmlImageCap *imgcap = GST_AMLIMGCAP (object);

  switch (prop_id) {
    case PROP_QUALITY:
      imgcap->quality = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_aml_imgcap_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAmlImageCap *imgcap = GST_AMLIMGCAP (object);

  switch (prop_id) {
    case PROP_QUALITY:
      g_value_set_int (value, imgcap->quality);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_aml_imgcap_set_caps (GstBaseTransform * base, GstCaps * in, GstCaps * out)
{
  GstAmlImageCap *imgcap = GST_AMLIMGCAP (base);
  GstVideoInfo in_info;

  if (!gst_video_info_from_caps (&in_info, in))
  {
    GST_ERROR_OBJECT (base, "caps are invalid");
    return FALSE;
  }
  imgcap->info = in_info;
  imgcap->is_info_set = TRUE;
  return TRUE;
}

#define GST_EVENT_CAPIMG GST_EVENT_MAKE_TYPE(81, GST_EVENT_TYPE_DOWNSTREAM | GST_EVENT_TYPE_SERIALIZED)
static gboolean
gst_aml_imgcap_sink_event (GstBaseTransform * base, GstEvent *event)
{
  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPIMG:
      {
        GstAmlImageCap *imgcap = GST_AMLIMGCAP (base);
        const GstStructure *resst = gst_event_get_structure (event);
        gboolean ret = TRUE;
        if (gst_structure_has_name (resst, "capture-image")) {
          GstMapInfo info;
          const GValue *value =
            gst_structure_get_value (resst, "location");

          if (imgcap->location) g_free (imgcap->location);
          imgcap->location = g_value_dup_string (value);

          imgcap->b_capreq = TRUE;

          return ret;
        }
      }
      break;
    default:
      break;
  }
  return GST_BASE_TRANSFORM_CLASS (parent_class)->sink_event (base, event);

}

/* GstBaseTransform vmethod implementations */

/* this function does the actual processing
 */
static GstFlowReturn
gst_aml_imgcap_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstAmlImageCap *imgcap = GST_AMLIMGCAP (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (GST_OBJECT (imgcap), GST_BUFFER_TIMESTAMP (outbuf));

  if (!imgcap->is_info_set) {
    GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL), ("unknown format"));
    return GST_FLOW_NOT_NEGOTIATED;
  }

  if (!imgcap->b_capreq) return GST_FLOW_OK;

  imgcap->b_capreq = FALSE;

  GstVideoInfo *info = &imgcap->info;
  GstMapInfo outbuf_info;
  if (gst_buffer_map (outbuf, &outbuf_info, GST_MAP_READ)) {
    write_JPEG_file (outbuf_info.data,
        info->width, info->height,
        imgcap->location, imgcap->quality);
    gst_buffer_unmap (outbuf, &outbuf_info);
  }

  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
amlimgcap_init (GstPlugin * amlimgcap)
{
  GST_DEBUG_CATEGORY_INIT (gst_aml_imgcap_debug, "amlimgcap", 0,
      "amlogic imgcap");

  return gst_element_register (amlimgcap, "amlimgcap", GST_RANK_PRIMARY,
      GST_TYPE_AMLIMGCAP);
}

/* gstreamer looks for this structure to register amlimgcaps
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amlimgcap,
    "Amlogic imgcap",
    amlimgcap_init,
    VERSION,
    "LGPL",
    "Amlogic image capture plugin",
    "http://openlinux.amlogic.com"
)
