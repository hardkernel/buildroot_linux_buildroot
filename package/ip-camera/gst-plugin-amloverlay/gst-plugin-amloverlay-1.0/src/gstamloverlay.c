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
 * SECTION:element-amloverlay
 *
 * FIXME:Describe amloverlay here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amloverlay ! fakesink silent=TRUE
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

#include "gstamloverlay.h"
#include "dfboverlay.h"

GST_DEBUG_CATEGORY_STATIC (gst_aml_overlay_debug);
#define GST_CAT_DEFAULT gst_aml_overlay_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

#define DEFAULT_PROP_DRAW_CLOCK TRUE
#define DEFAULT_PROP_DRAW_PTS FALSE
#define DEFAULT_PROP_DRAW_OUTLINE TRUE
#define DEFAULT_PROP_CLOCK_POS GST_AML_TEXT_OVERLAY_POS_TOP_RIGHT
#define DEFAULT_PROP_PTS_POS GST_AML_TEXT_OVERLAY_POS_TOP_LEFT
#define DEFAULT_PROP_FONT_FILE "/usr/share/directfb-1.7.7/decker.ttf"
#define DEFAULT_PROP_FONT_SIZE 48
#define DEFAULT_PROP_FONT_BGCOLOR 0x00000000
#define DEFAULT_PROP_FONT_COLOR 0xffffffff
#define DEFAULT_DRAW_OUTLINE FALSE
#define DEFAULT_PROP_WATERMARK_TEXT ""
#define DEFAULT_PROP_WATERMARK_TEXT_FONT_FILE DEFAULT_PROP_FONT_FILE
#define DEFAULT_PROP_WATERMARK_TEXT_FONT_COLOR 0xffffff80
#define DEFAULT_PROP_WATERMARK_TEXT_FONT_SIZE 64
#define DEFAULT_PROP_WATERMARK_TEXT_XPOS 128
#define DEFAULT_PROP_WATERMARK_TEXT_YPOS 128
#define DEFAULT_PROP_WATERMARK_IMG ""
#define DEFAULT_PROP_WATERMARK_IMG_XPOS 0
#define DEFAULT_PROP_WATERMARK_IMG_YPOS 0
#define DEFAULT_PROP_WATERMARK_IMG_WIDTH -1
#define DEFAULT_PROP_WATERMARK_IMG_HEIGHT -1
#define DEFAULT_PROP_DISABLE_FACERECT FALSE
#define DEFAULT_PROP_FACERECT_COLOR 0xffff00ff

enum
{
  PROP_0,
  PROP_DRAW_CLOCK,
  PROP_CLOCK_POS,
  PROP_DRAW_PTS,
  PROP_PTS_POS,
  PROP_FONT_FILE,
  PROP_FONT_COLOR,
  PROP_FONT_BGCOLOR,
  PROP_FONT_SIZE,
  PROP_DRAW_OUTLINE,
  PROP_WATERMARK_TEXT,
  PROP_WATERMARK_TEXT_FONT_FILE,
  PROP_WATERMARK_TEXT_FONT_COLOR,
  PROP_WATERMARK_TEXT_FONT_SIZE,
  PROP_WATERMARK_TEXT_XPOS,
  PROP_WATERMARK_TEXT_YPOS,
  PROP_WATERMARK_IMG,
  PROP_WATERMARK_IMG_XPOS,
  PROP_WATERMARK_IMG_YPOS,
  PROP_WATERMARK_IMG_WIDTH,
  PROP_WATERMARK_IMG_HEIGHT,
  PROP_DISABLE_FACERECT,
  PROP_FACERECT_COLOR,
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

#define gst_aml_overlay_parent_class parent_class
G_DEFINE_TYPE (GstAmlOverlay, gst_aml_overlay, GST_TYPE_BASE_TRANSFORM);

static void gst_aml_overlay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_aml_overlay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_aml_overlay_open (GstAmlOverlay * base);

static gboolean gst_aml_overlay_close (GstAmlOverlay * base);

static void gst_aml_overlay_finalize (GObject * object);

static GstFlowReturn gst_aml_overlay_transform_ip (GstBaseTransform * base,
    GstBuffer * outbuf);

static gboolean gst_aml_overlay_set_caps (GstBaseTransform * base,
    GstCaps * in, GstCaps * out);

static gboolean gst_aml_overlay_sink_event (GstBaseTransform * base, GstEvent *event);
/* GObject vmethod implementations */

#define GST_TYPE_AML_TEXT_OVERLAY_POS (gst_aml_text_overlay_pos_get_type())
static GType
gst_aml_text_overlay_pos_get_type (void)
{
  static GType aml_text_overlay_pos_type = 0;
  static const GEnumValue aml_text_overlay_pos[] = {
    {GST_AML_TEXT_OVERLAY_POS_TOP_LEFT, "top-left", "top left"},
    {GST_AML_TEXT_OVERLAY_POS_TOP_MID, "top-mid", "top middle"},
    {GST_AML_TEXT_OVERLAY_POS_TOP_RIGHT, "top-right", "top right"},
    {GST_AML_TEXT_OVERLAY_POS_MID_LEFT, "mid-left", "middle left"},
    {GST_AML_TEXT_OVERLAY_POS_MID_RIGHT, "mid-right", "middle right"},
    {GST_AML_TEXT_OVERLAY_POS_CENTER, "center", "center"},
    {GST_AML_TEXT_OVERLAY_POS_BOTTOM_LEFT, "bot-left", "bottom left"},
    {GST_AML_TEXT_OVERLAY_POS_BOTTOM_MID, "bot-mid", "bottom middle"},
    {GST_AML_TEXT_OVERLAY_POS_BOTTOM_RIGHT, "bot-right", "bottom right"},
    {0, NULL, NULL},
  };

  if (!aml_text_overlay_pos_type) {
    aml_text_overlay_pos_type =
        g_enum_register_static ("GstAMLTextOverlayPos",
        aml_text_overlay_pos);
  }
  return aml_text_overlay_pos_type;
}

/* initialize the amloverlay's class */
static void
gst_aml_overlay_class_init (GstAmlOverlayClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseTransformClass *gstbasetransform_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasetransform_class = (GstBaseTransformClass *)klass;


  gobject_class->set_property = gst_aml_overlay_set_property;
  gobject_class->get_property = gst_aml_overlay_get_property;
  gobject_class->finalize = gst_aml_overlay_finalize;
  gstbasetransform_class->sink_event = gst_aml_overlay_sink_event;

  g_object_class_install_property (gobject_class, PROP_DRAW_CLOCK,
      g_param_spec_boolean ("draw-clock", "Draw-Clock",
          "Whether to draw clock",
          DEFAULT_PROP_DRAW_CLOCK,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_DRAW_PTS,
      g_param_spec_boolean ("draw-pts", "Draw-PTS",
          "Whether to draw buffer pts",
          DEFAULT_PROP_DRAW_PTS,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_CLOCK_POS,
      g_param_spec_enum ("clock-pos", "clock-pos",
          "clock position", GST_TYPE_AML_TEXT_OVERLAY_POS,
          DEFAULT_PROP_CLOCK_POS, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_PTS_POS,
      g_param_spec_enum ("pts-pos", "pts-pos",
          "pts position", GST_TYPE_AML_TEXT_OVERLAY_POS,
          DEFAULT_PROP_PTS_POS, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_DISABLE_FACERECT,
      g_param_spec_boolean ("disable-face", "disable-face",
          "Whether to disable face detection result",
          DEFAULT_PROP_DISABLE_FACERECT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FONT_FILE,
      g_param_spec_string ("font-file", "Font-File",
        "Truetype font file for display pts/clock", DEFAULT_PROP_FONT_FILE,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FONT_SIZE,
      g_param_spec_int ("font-size", "Font-Size",
        "Font size for pts/clock", 8,
        256, DEFAULT_PROP_FONT_SIZE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FONT_COLOR,
      g_param_spec_uint ("font-color", "Font-Color",
        "Color to use for text (RGBA).", 0, G_MAXUINT32,
        DEFAULT_PROP_FONT_COLOR,
        G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FONT_BGCOLOR,
      g_param_spec_uint ("font-background-color", "Font-Background-Color",
        "Background Color to use for text (RGBA).", 0, G_MAXUINT32,
        DEFAULT_PROP_FONT_BGCOLOR,
        G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_DRAW_OUTLINE,
      g_param_spec_boolean ("draw-outline", "Draw-Outline",
          "Whether to draw text outline",
          DEFAULT_PROP_DRAW_OUTLINE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_TEXT,
      g_param_spec_string ("watermark-text", "Watermark-Text",
        "Watermark text to be display.", DEFAULT_PROP_WATERMARK_TEXT,
        G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_TEXT_XPOS,
      g_param_spec_int ("watermark-text-xpos", "Watermark-Text-XPos",
        "Horizontal start position of watermark text", 0,
        G_MAXINT32, DEFAULT_PROP_WATERMARK_TEXT_XPOS, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_TEXT_YPOS,
      g_param_spec_int ("watermark-text-ypos", "Watermark-Text-YPos",
        "Vertical start position of watermark text", 0,
        G_MAXINT32, DEFAULT_PROP_WATERMARK_TEXT_YPOS, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_IMG,
      g_param_spec_string ("watermark-image", "Watermark-Image",
        "Path of watermark image to be display.", DEFAULT_PROP_WATERMARK_IMG,
        G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_IMG_XPOS,
      g_param_spec_int ("watermark-image-xpos", "Watermark-Image-XPos",
        "Horizontal start position of watermark image", 0,
        G_MAXINT32, DEFAULT_PROP_WATERMARK_IMG_XPOS, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_IMG_YPOS,
      g_param_spec_int ("watermark-image-ypos", "Watermark-Image-YPos",
        "Vertical start position of watermark image", 0,
        G_MAXINT32, DEFAULT_PROP_WATERMARK_IMG_YPOS, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_IMG_WIDTH,
      g_param_spec_int ("watermark-image-width", "Watermark-Image-Width",
        "Width of watermark image (-1 for auto adjusting)", -1,
        G_MAXINT32, DEFAULT_PROP_WATERMARK_IMG_WIDTH, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_IMG_HEIGHT,
      g_param_spec_int ("watermark-image-height", "Watermark-Image-Height",
        "Height of watermark image (-1 for auto adjusting)", -1,
        G_MAXINT32, DEFAULT_PROP_WATERMARK_IMG_HEIGHT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_TEXT_FONT_FILE,
      g_param_spec_string ("watermark-font-file", "Watermark-Font-File",
        "Truetype font file for display wartermark", DEFAULT_PROP_WATERMARK_TEXT_FONT_FILE,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_TEXT_FONT_SIZE,
      g_param_spec_int ("watermark-font-size", "Watermark-Font-Size",
        "Font size for watermark", 8,
        512, DEFAULT_PROP_WATERMARK_TEXT_FONT_SIZE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_WATERMARK_TEXT_FONT_COLOR,
      g_param_spec_uint ("watermark-font-color", "Watermark-Font-Color",
        "Color to use for watermark text (RGBA).", 0, G_MAXUINT32,
        DEFAULT_PROP_WATERMARK_TEXT_FONT_COLOR,
        G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FACERECT_COLOR,
      g_param_spec_uint ("face-rect-color", "Face-Rect-Color",
        "Color to use for face detection rectangel (RGBA).", 0, G_MAXUINT32,
        DEFAULT_PROP_FACERECT_COLOR,
        G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

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
      GST_DEBUG_FUNCPTR (gst_aml_overlay_transform_ip);

  GST_BASE_TRANSFORM_CLASS (klass)->set_caps =
      GST_DEBUG_FUNCPTR (gst_aml_overlay_set_caps);

}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_aml_overlay_init (GstAmlOverlay *overlay)
{
  overlay->is_info_set = FALSE;

  overlay->clock_font = NULL;
  overlay->clock_surface = NULL;
  overlay->pts_font = NULL;
  overlay->pts_surface = NULL;
  overlay->watermark_font = NULL;
  overlay->watermark_text_surface = NULL;
  overlay->watermark_img_surface = NULL;

  overlay->draw_clock = DEFAULT_PROP_DRAW_CLOCK;
  overlay->draw_pts = DEFAULT_PROP_DRAW_PTS;
  overlay->clock_pos = DEFAULT_PROP_CLOCK_POS;
  overlay->pts_pos = DEFAULT_PROP_PTS_POS;

  overlay->fontfile = DEFAULT_PROP_FONT_FILE;
  overlay->fontsize = DEFAULT_PROP_FONT_SIZE;
  overlay->fontcolor = DEFAULT_PROP_FONT_COLOR;
  overlay->bgcolor = DEFAULT_PROP_FONT_BGCOLOR;

  overlay->draw_outline = DEFAULT_DRAW_OUTLINE;
  overlay->watermark_text = g_strdup(DEFAULT_PROP_WATERMARK_TEXT);
  overlay->watermark_fontfile = g_strdup(DEFAULT_PROP_WATERMARK_TEXT_FONT_FILE);
  overlay->watermark_fontcolor = DEFAULT_PROP_WATERMARK_TEXT_FONT_COLOR;
  overlay->watermark_fontsize = DEFAULT_PROP_WATERMARK_TEXT_FONT_SIZE;
  overlay->watermark_text_xpos = DEFAULT_PROP_WATERMARK_TEXT_XPOS;
  overlay->watermark_text_ypos = DEFAULT_PROP_WATERMARK_TEXT_YPOS;
  overlay->watermark_img = g_strdup(DEFAULT_PROP_WATERMARK_IMG);
  overlay->watermark_img_xpos = DEFAULT_PROP_WATERMARK_IMG_XPOS;
  overlay->watermark_img_ypos = DEFAULT_PROP_WATERMARK_IMG_YPOS;
  overlay->watermark_img_width = DEFAULT_PROP_WATERMARK_IMG_WIDTH;
  overlay->watermark_img_height = DEFAULT_PROP_WATERMARK_IMG_HEIGHT;
  overlay->disable_facerect = DEFAULT_PROP_DISABLE_FACERECT;
  overlay->facerect_color = DEFAULT_PROP_FACERECT_COLOR;

  gst_aml_overlay_open (overlay);
}

static void
gst_aml_overlay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmlOverlay *overlay = GST_AMLOVERLAY (object);

  switch (prop_id) {
    case PROP_DRAW_CLOCK:
      overlay->draw_clock = g_value_get_boolean (value);
      break;
    case PROP_DRAW_PTS:
      overlay->draw_pts = g_value_get_boolean (value);
      break;
    case PROP_CLOCK_POS:
      overlay->clock_pos = g_value_get_enum (value);
      break;
    case PROP_PTS_POS:
      overlay->pts_pos = g_value_get_enum (value);
      break;
    case PROP_FONT_FILE:
      g_free(overlay->fontfile);
      overlay->fontfile = g_value_dup_string (value);
      break;
    case PROP_FONT_COLOR:
      overlay->fontcolor = g_value_get_uint (value);
      break;
    case PROP_FONT_BGCOLOR:
      overlay->bgcolor = g_value_get_uint (value);
      break;
    case PROP_FONT_SIZE:
      overlay->fontsize = g_value_get_int (value);
      break;
    case PROP_DRAW_OUTLINE:
      overlay->draw_outline = g_value_get_boolean (value);
      break;
    case PROP_WATERMARK_TEXT:
      g_free(overlay->watermark_text);
      overlay->watermark_text = g_value_dup_string (value);
      break;
    case PROP_WATERMARK_TEXT_FONT_FILE:
      g_free(overlay->watermark_fontfile);
      overlay->watermark_fontfile = g_value_dup_string (value);
      break;
    case PROP_WATERMARK_TEXT_FONT_COLOR:
      overlay->watermark_fontcolor = g_value_get_uint (value);
      break;
    case PROP_WATERMARK_TEXT_FONT_SIZE:
      overlay->watermark_fontsize = g_value_get_int (value);
      break;
    case PROP_WATERMARK_TEXT_XPOS:
      overlay->watermark_text_xpos = g_value_get_int (value);
      break;
    case PROP_WATERMARK_TEXT_YPOS:
      overlay->watermark_text_ypos = g_value_get_int (value);
      break;
    case PROP_WATERMARK_IMG:
      g_free(overlay->watermark_img);
      overlay->watermark_img = g_value_dup_string (value);
      break;
    case PROP_WATERMARK_IMG_XPOS:
      overlay->watermark_img_xpos = g_value_get_int (value);
      break;
    case PROP_WATERMARK_IMG_YPOS:
      overlay->watermark_img_ypos = g_value_get_int (value);
      break;
    case PROP_WATERMARK_IMG_WIDTH:
      overlay->watermark_img_width = g_value_get_int (value);
      break;
    case PROP_WATERMARK_IMG_HEIGHT:
      overlay->watermark_img_height = g_value_get_int (value);
      break;
    case PROP_DISABLE_FACERECT:
      overlay->disable_facerect = g_value_get_boolean (value);
      break;
    case PROP_FACERECT_COLOR:
      overlay->facerect_color = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_aml_overlay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAmlOverlay *overlay = GST_AMLOVERLAY (object);

  switch (prop_id) {
    case PROP_DRAW_CLOCK:
      g_value_set_boolean (value, overlay->draw_clock);
      break;
    case PROP_DRAW_PTS:
      g_value_set_boolean (value, overlay->draw_pts);
      break;
    case PROP_CLOCK_POS:
      g_value_set_enum (value, overlay->clock_pos);
      break;
    case PROP_PTS_POS:
      g_value_set_enum (value, overlay->pts_pos);
      break;
    case PROP_FONT_FILE:
      g_value_set_string (value, overlay->fontfile);
      break;
    case PROP_FONT_COLOR:
      g_value_set_uint (value, overlay->fontcolor);
      break;
    case PROP_FONT_BGCOLOR:
      g_value_set_uint (value, overlay->bgcolor);
      break;
    case PROP_FONT_SIZE:
      g_value_set_int (value, overlay->fontsize);
      break;
    case PROP_DRAW_OUTLINE:
      g_value_set_boolean (value, overlay->draw_outline);
      break;
    case PROP_WATERMARK_TEXT:
      g_value_set_string (value, overlay->watermark_text);
      break;
    case PROP_WATERMARK_TEXT_FONT_FILE:
      g_value_set_string (value, overlay->watermark_fontfile);
      break;
    case PROP_WATERMARK_TEXT_FONT_COLOR:
      g_value_set_uint (value, overlay->watermark_fontcolor);
      break;
    case PROP_WATERMARK_TEXT_FONT_SIZE:
      g_value_set_int (value, overlay->watermark_fontsize);
      break;
    case PROP_WATERMARK_TEXT_XPOS:
      g_value_set_int (value, overlay->watermark_text_xpos);
      break;
    case PROP_WATERMARK_TEXT_YPOS:
      g_value_set_int (value, overlay->watermark_text_ypos);
      break;
    case PROP_WATERMARK_IMG:
      g_value_set_string (value, overlay->watermark_img);
      break;
    case PROP_WATERMARK_IMG_XPOS:
      g_value_set_int (value, overlay->watermark_img_xpos);
      break;
    case PROP_WATERMARK_IMG_YPOS:
      g_value_set_int (value, overlay->watermark_img_ypos);
      break;
    case PROP_WATERMARK_IMG_WIDTH:
      g_value_set_int (value, overlay->watermark_img_width);
      break;
    case PROP_WATERMARK_IMG_HEIGHT:
      g_value_set_int (value, overlay->watermark_img_height);
      break;
    case PROP_DISABLE_FACERECT:
      g_value_set_boolean (value, overlay->disable_facerect);
      break;
    case PROP_FACERECT_COLOR:
      g_value_set_uint (value, overlay->facerect_color);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_aml_overlay_open (GstAmlOverlay * overlay)
{
  return TRUE;
}

static gboolean
gst_aml_overlay_close (GstAmlOverlay * overlay)
{
  if (overlay->clock_font) {
    overlay_destroy_font (overlay->clock_font);
    overlay->clock_font = overlay->pts_font = NULL;
  }
  if (overlay->watermark_font) {
    overlay_destroy_font (overlay->watermark_font);
    overlay->watermark_font = NULL;
  }
  if (overlay->watermark_text_surface) {
    overlay_destroy_surface (overlay->watermark_text_surface);
    overlay->watermark_text_surface = NULL;
  }
  if (overlay->watermark_img_surface) {
    overlay_destroy_surface (overlay->watermark_img_surface);
    overlay->watermark_img_surface = NULL;
  }
  overlay_deinit();
  return TRUE;
}

static void
gst_aml_overlay_finalize (GObject * object)
{
  GstAmlOverlay *overlay = GST_AMLOVERLAY (object);

  gst_aml_overlay_close (overlay);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static gboolean
gst_aml_overlay_set_caps (GstBaseTransform * base, GstCaps * in, GstCaps * out)
{
  GstAmlOverlay *overlay = GST_AMLOVERLAY (base);
  GstVideoInfo in_info;

  if (!gst_video_info_from_caps (&in_info, in))
  {
    GST_ERROR_OBJECT (base, "caps are invalid");
    return FALSE;
  }
  overlay->info = in_info;
  overlay->is_info_set = TRUE;
  return TRUE;
}

#define MAX_DETECT_NUM 100
typedef struct _DetectPoint {
  int left;
  int top;
  int right;
  int bottom;
} DetectPoint;

typedef struct _DetectResult {
   int  detect_num;
   DetectPoint pt[MAX_DETECT_NUM];
} DetectResult;

#define GST_EVENT_YOLOFACE_DETECTED GST_EVENT_MAKE_TYPE(80, GST_EVENT_TYPE_DOWNSTREAM | GST_EVENT_TYPE_SERIALIZED)
DetectResult *face_detect_result = NULL;
static gboolean
gst_aml_overlay_sink_event (GstBaseTransform * base, GstEvent *event)
{
  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_YOLOFACE_DETECTED:
      {
        const GstStructure *resst = gst_event_get_structure (event);
        gboolean ret = TRUE;
        if (gst_structure_has_name (resst, "face-detection")) {
          GstMapInfo info;
          const GValue *size = gst_structure_get_value (resst, "rectnum");
          const GValue *buf = gst_structure_get_value (resst, "rectbuf");
          GstBuffer *resbuf = gst_value_get_buffer (buf);
          DetectResult *res = (DetectResult *)g_malloc (sizeof(DetectResult));
          res->detect_num = g_value_get_int (size);
          if (gst_buffer_map (resbuf, &info, GST_MAP_READ)) {
            g_memmove (res->pt, info.data, info.size);
            gst_buffer_unmap (resbuf, &info);
          } else {
            g_free (res);
            res = NULL;
            ret = FALSE;
          }

          if (face_detect_result) {
            g_free (face_detect_result);
          }
          face_detect_result = res;
          gst_buffer_unref (resbuf);

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

static void
gst_aml_overlay_calc_text_pos(gint string_width, gint font_height,
    gint width, gint height, GstAmlTextOverlayPOS pos,
    gint* x, gint* y) {

  gint xpos, ypos;
  gint hvpad = font_height / 2;
  if (hvpad > 20) hvpad = 20;
  switch (pos) {
    case GST_AML_TEXT_OVERLAY_POS_TOP_LEFT:
      xpos = hvpad; ypos = hvpad;
      break;

    case GST_AML_TEXT_OVERLAY_POS_TOP_MID:
      xpos = (width - string_width) / 2;
      ypos = hvpad;
      break;

    case GST_AML_TEXT_OVERLAY_POS_TOP_RIGHT:
      xpos = width - string_width - hvpad;
      ypos = hvpad;
      break;

    case GST_AML_TEXT_OVERLAY_POS_MID_LEFT:
      xpos = hvpad;
      ypos = (height - font_height) / 2;
      break;

    case GST_AML_TEXT_OVERLAY_POS_MID_RIGHT:
      xpos = width - string_width - hvpad;
      ypos = (height - font_height) / 2;
      break;

    case GST_AML_TEXT_OVERLAY_POS_CENTER:
      xpos = (width - string_width) / 2;
      ypos = (height - font_height) / 2;
      break;

    case GST_AML_TEXT_OVERLAY_POS_BOTTOM_LEFT:
      xpos = hvpad;
      ypos = height - font_height - hvpad;
      break;

    case GST_AML_TEXT_OVERLAY_POS_BOTTOM_MID:
      xpos = (width - string_width) / 2;
      ypos = height - font_height - hvpad;
      break;

    case GST_AML_TEXT_OVERLAY_POS_BOTTOM_RIGHT:
      xpos = width - string_width - hvpad;
      ypos = height - font_height - hvpad;
      break;
    default:
      xpos = hvpad; ypos = hvpad;
      break;
  }

  if (xpos < 0 || xpos > width) xpos = 0;
  if (ypos < 0 || ypos > height) ypos = 0;

  if (x) *x = xpos;
  if (y) *y = ypos;
}

static gchar *
gst_aml_overlay_clock_time ()
{
  struct tm *t;
  time_t now;
  gchar buf[256];

  now = time (NULL);

  t = localtime (&now);

  if (t == NULL)
    return g_strdup ("--:--:--");

  if (strftime (buf, sizeof (buf), "%H:%M:%S", t) == 0)
    return g_strdup ("");
  return g_strdup (buf);
}

static gchar *
gst_aml_overlay_pts_time (GstClockTime time)
{
  guint hours, mins, secs, msecs;

  if (!GST_CLOCK_TIME_IS_VALID (time))
    return g_strdup ("");

  hours = (guint) (time / (GST_SECOND * 60 * 60));
  mins = (guint) ((time / (GST_SECOND * 60)) % 60);
  secs = (guint) ((time / GST_SECOND) % 60);
  msecs = (guint) ((time % GST_SECOND) / (1000 * 1000));

  return g_strdup_printf ("%u:%02u:%02u.%03u", hours, mins, secs, msecs);
}
/* this function does the actual processing
 */
static GstFlowReturn
gst_aml_overlay_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstAmlOverlay *overlay = GST_AMLOVERLAY (base);
  gint x, y;

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (GST_OBJECT (overlay), GST_BUFFER_TIMESTAMP (outbuf));

  if (!overlay->is_info_set) {
    GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL), ("unknown format"));
    return GST_FLOW_NOT_NEGOTIATED;
  }

  GstVideoInfo *info = &overlay->info;
  GstMapInfo outbuf_info;
  if (gst_buffer_map (outbuf, &outbuf_info, GST_MAP_READ | GST_MAP_WRITE)) {
    overlay_init();
    overlay_create_inputbuffer (outbuf_info.data, info->width, info->height);
    if (overlay->draw_clock || overlay->draw_pts) {
      if (overlay->clock_font == NULL) {
        overlay->clock_font = overlay->pts_font = overlay_create_font (overlay->fontfile,
            overlay->fontsize, overlay->draw_outline ? 1 : 0);
      }
      if (overlay->draw_clock) {
        gchar* txt = gst_aml_overlay_clock_time();
        gint strw, strh;
        overlay_get_string_wh (txt, overlay->clock_font, &strw, &strh);
        gst_aml_overlay_calc_text_pos (strw, strh, info->width, info->height,
            overlay->clock_pos, &x, &y);
        overlay->clock_surface = overlay_create_text_surface (txt, overlay->clock_font, overlay->fontcolor, overlay->bgcolor);
        g_free(txt);
        overlay_draw_surface (overlay->clock_surface, x, y);
        overlay_destroy_surface (overlay->clock_surface);
        overlay->clock_surface = NULL;
      }
      if (overlay->draw_pts) {
        GstClockTime ts_buffer;

        ts_buffer = GST_BUFFER_TIMESTAMP (outbuf);

        if (GST_CLOCK_TIME_IS_VALID (ts_buffer)) {
          gchar* txt = gst_aml_overlay_pts_time (ts_buffer);
          gint strw, strh;
          overlay_get_string_wh (txt, overlay->pts_font, &strw, &strh);
          gst_aml_overlay_calc_text_pos (strw, strh, info->width, info->height,
              overlay->pts_pos, &x, &y);
          overlay->pts_surface = overlay_create_text_surface (txt, overlay->clock_font, overlay->fontcolor, overlay->bgcolor);
          g_free (txt);
          overlay_draw_surface (overlay->pts_surface, x, y);
          overlay_destroy_surface (overlay->pts_surface);
          overlay->pts_surface = NULL;
		}
      }
    }
    if (strlen (overlay->watermark_text) > 0) {
      if (overlay->watermark_font == NULL) {
        overlay->watermark_font = overlay_create_font (overlay->watermark_fontfile,
            overlay->watermark_fontsize, 0);
      }
      if (overlay->watermark_text_surface == NULL) {
        overlay->watermark_text_surface = overlay_create_text_surface (overlay->watermark_text,
            overlay->watermark_font, overlay->watermark_fontcolor, 0);
      }
      overlay_draw_surface (overlay->watermark_text_surface,
          overlay->watermark_text_xpos, overlay->watermark_text_ypos);
    }

    if (strlen (overlay->watermark_img) > 0) {
      if (overlay->watermark_img_surface == NULL) {
        overlay->watermark_img_surface = overlay_create_image_surface (overlay->watermark_img,
            overlay->watermark_img_width, overlay->watermark_img_height);
      }
      overlay_draw_surface (overlay->watermark_img_surface,
          overlay->watermark_img_xpos, overlay->watermark_img_ypos);
    }
    if (!overlay->disable_facerect) {
      if (face_detect_result) {
        for (int i = 0; i < face_detect_result->detect_num; i++) {
          DetectPoint *pt = &face_detect_result->pt[i];
          overlay_draw_rect(pt->left, pt->top,
              pt->right - pt->left, pt->bottom - pt->top, 3,
              overlay->facerect_color);
        }
        g_free(face_detect_result);
        face_detect_result = NULL;
      }
    }
    overlay_destroy_inputbuffer();
    gst_buffer_unmap (outbuf, &outbuf_info);
  }

  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
amloverlay_init (GstPlugin * amloverlay)
{
  GST_DEBUG_CATEGORY_INIT (gst_aml_overlay_debug, "amloverlay", 0,
      "amlogic overlay");

  return gst_element_register (amloverlay, "amloverlay", GST_RANK_PRIMARY,
      GST_TYPE_AMLOVERLAY);
}

/* gstreamer looks for this structure to register amloverlays
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amloverlay,
    "Amlogic overlay",
    amloverlay_init,
    VERSION,
    "LGPL",
    "Amlogic overlay plugin",
    "http://openlinux.amlogic.com"
)
