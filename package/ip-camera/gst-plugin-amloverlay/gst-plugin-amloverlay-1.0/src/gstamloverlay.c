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

#define DETECT_DELAY_FRAMES 5

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
#define DEFAULT_PROP_NNRECT_SHOW TRUE
#define DEFAULT_PROP_NN_RECTCOLOR 0xffff00ff

#define DEFAULT_PROP_FACENET_SHOW TRUE
#define DEFAULT_PROP_FACENET_FONTCOLOR 0x00ff80ff
#define DEFAULT_PROP_FACENET_FONTFILE "/usr/share/directfb-1.7.7/decker.ttf"
#define DEFAULT_PROP_FACENET_FONTSIZE 32
#define DEFAULT_PROP_FACENET_RECTCOLOR 0xff00ffff

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
  PROP_NNRECT_SHOW,
  PROP_NN_RECTCOLOR,

  PROP_FACENET_SHOW,
  PROP_FACENET_FONTCOLOR,
  PROP_FACENET_FONTFILE,
  PROP_FACENET_FONTSIZE,
  PROP_FACENET_RECTCOLOR,
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

struct RelativePos {
  float x0;
  float y0;
  float x1;
  float y1;
};

struct NNResult {
  struct listnode list;
  guint64 frameidx;
  gint detect_num;
  struct RelativePos *pt;
};

struct FaceNetEventBuffer {
  struct RelativePos pos;
  char *info;
};

struct FaceNetResult {
  struct listnode list;
  guint64 frameidx;
  struct FaceNetEventBuffer *data;
};

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

static gboolean gst_aml_overlay_event (GstBaseTransform * base, GstEvent *event);

static void cleanup_nn_list (GstAmlOverlay *overlay);
static void cleanup_facenet_list (GstAmlOverlay *overlay, guint64 frameidx);
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

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;


  gobject_class->set_property = gst_aml_overlay_set_property;
  gobject_class->get_property = gst_aml_overlay_get_property;
  gobject_class->finalize = gst_aml_overlay_finalize;

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

  g_object_class_install_property (gobject_class, PROP_NNRECT_SHOW,
      g_param_spec_boolean ("show-nn-rect", "show-nn-rect",
          "show nn detection rect",
          DEFAULT_PROP_NNRECT_SHOW,
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

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_NN_RECTCOLOR,
      g_param_spec_uint ("nn-rect-color", "NN-Rect-Color",
        "Color to use for nn detection rectangel (RGBA).", 0, G_MAXUINT32,
        DEFAULT_PROP_NN_RECTCOLOR,
        G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_FACENET_SHOW,
      g_param_spec_boolean ("show-facenet", "show-facenet",
          "show facenet detection info",
          DEFAULT_PROP_FACENET_SHOW,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FACENET_FONTFILE,
      g_param_spec_string ("facenet-font-file", "Facenet-Font-File",
        "Truetype font file for display facenet info", DEFAULT_PROP_FACENET_FONTFILE,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FACENET_FONTSIZE,
      g_param_spec_int ("facenet-font-size", "Facenet-Font-Size",
        "Font size for facenet info", 8,
        256, DEFAULT_PROP_FACENET_FONTSIZE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FACENET_FONTCOLOR,
      g_param_spec_uint ("facenet-font-color", "Facenet-Font-Color",
        "Color to use for facenet info (RGBA).", 0, G_MAXUINT32,
        DEFAULT_PROP_FACENET_FONTCOLOR,
        G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FACENET_RECTCOLOR,
      g_param_spec_uint ("facenet-rect-color", "Facenet-Rect-Color",
        "Color to use for facenet rectangel (RGBA).", 0, G_MAXUINT32,
        DEFAULT_PROP_FACENET_RECTCOLOR,
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

  GST_BASE_TRANSFORM_CLASS (klass)->sink_event =
      GST_DEBUG_FUNCPTR (gst_aml_overlay_event);

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

  overlay->fontfile = g_strdup(DEFAULT_PROP_FONT_FILE);
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
  overlay->nnrect_show = DEFAULT_PROP_NNRECT_SHOW;
  overlay->nn_rectcolor = DEFAULT_PROP_NN_RECTCOLOR;

  overlay->facenet_show = DEFAULT_PROP_FACENET_SHOW;
  overlay->facenet_rectcolor = DEFAULT_PROP_FACENET_RECTCOLOR;
  overlay->facenet_fontfile = g_strdup(DEFAULT_PROP_FACENET_FONTFILE);
  overlay->facenet_fontcolor = DEFAULT_PROP_FACENET_FONTCOLOR;
  overlay->facenet_fontsize = DEFAULT_PROP_FACENET_FONTSIZE;

  overlay->font_changed = FALSE;
  overlay->watermark_text_font_changed = FALSE;
  overlay->watermark_text_changed = FALSE;
  overlay->watermark_img_changed = FALSE;

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
      overlay->font_changed = TRUE;
      break;
    case PROP_FONT_COLOR:
      overlay->fontcolor = g_value_get_uint (value);
      break;
    case PROP_FONT_BGCOLOR:
      overlay->bgcolor = g_value_get_uint (value);
      break;
    case PROP_FONT_SIZE:
      overlay->fontsize = g_value_get_int (value);
      overlay->font_changed = TRUE;
      break;
    case PROP_DRAW_OUTLINE:
      overlay->draw_outline = g_value_get_boolean (value);
      break;
    case PROP_WATERMARK_TEXT:
      {
        gchar *t = g_value_dup_string (value);
        if (g_strcmp0 (t, overlay->watermark_text)) {
          g_free (overlay->watermark_text);
          overlay->watermark_text = t;
          overlay->watermark_text_changed = TRUE;
        } else {
          g_free (t);
        }
      }
      break;
    case PROP_WATERMARK_TEXT_FONT_FILE:
      g_free(overlay->watermark_fontfile);
      overlay->watermark_fontfile = g_value_dup_string (value);
      overlay->watermark_text_font_changed = TRUE;
      break;
    case PROP_WATERMARK_TEXT_FONT_COLOR:
      overlay->watermark_fontcolor = g_value_get_uint (value);
      break;
    case PROP_WATERMARK_TEXT_FONT_SIZE:
      overlay->watermark_fontsize = g_value_get_int (value);
      overlay->watermark_text_font_changed = TRUE;
      break;
    case PROP_WATERMARK_TEXT_XPOS:
      overlay->watermark_text_xpos = g_value_get_int (value);
      break;
    case PROP_WATERMARK_TEXT_YPOS:
      overlay->watermark_text_ypos = g_value_get_int (value);
      break;
    case PROP_WATERMARK_IMG:
      {
        gchar *t = g_value_dup_string (value);
        if (g_strcmp0 (t, overlay->watermark_img)) {
          g_free (overlay->watermark_img);
          overlay->watermark_img = t;
          overlay->watermark_img_changed = TRUE;
        } else {
          g_free (t);
        }
      }
      break;
    case PROP_WATERMARK_IMG_XPOS:
      overlay->watermark_img_xpos = g_value_get_int (value);
      break;
    case PROP_WATERMARK_IMG_YPOS:
      overlay->watermark_img_ypos = g_value_get_int (value);
      break;
    case PROP_WATERMARK_IMG_WIDTH:
      {
        gint i = g_value_get_int (value);
        if (i != overlay->watermark_img_width) {
          overlay->watermark_img_width = i;
          overlay->watermark_img_changed = TRUE;
        }
      }
      break;
    case PROP_WATERMARK_IMG_HEIGHT:
      {
        gint i = g_value_get_int (value);
        if (i != overlay->watermark_img_height) {
          overlay->watermark_img_height = i;
          overlay->watermark_img_changed = TRUE;
        }
      }
      break;
    case PROP_NNRECT_SHOW:
      overlay->nnrect_show = g_value_get_boolean (value);
      break;
    case PROP_NN_RECTCOLOR:
      overlay->nn_rectcolor = g_value_get_uint (value);
      break;
    case PROP_FACENET_SHOW:
      overlay->facenet_show = g_value_get_boolean (value);
      break;
    case PROP_FACENET_RECTCOLOR:
      overlay->facenet_rectcolor = g_value_get_uint (value);
      break;
    case PROP_FACENET_FONTFILE:
      g_free(overlay->facenet_fontfile);
      overlay->facenet_fontfile = g_value_dup_string (value);
      overlay->facenet_font_changed = TRUE;
      break;
    case PROP_FACENET_FONTCOLOR:
      overlay->facenet_fontcolor = g_value_get_uint (value);
      break;
    case PROP_FACENET_FONTSIZE:
      overlay->facenet_fontsize = g_value_get_int (value);
      overlay->facenet_font_changed = TRUE;
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
    case PROP_NNRECT_SHOW:
      g_value_set_boolean (value, overlay->nnrect_show);
      break;
    case PROP_NN_RECTCOLOR:
      g_value_set_uint (value, overlay->nn_rectcolor);
      break;
    case PROP_FACENET_SHOW:
      g_value_set_boolean (value, overlay->facenet_show);
      break;
    case PROP_FACENET_RECTCOLOR:
      g_value_set_uint (value, overlay->facenet_rectcolor);
      break;
    case PROP_FACENET_FONTFILE:
      g_value_set_string (value, overlay->facenet_fontfile);
      break;
    case PROP_FACENET_FONTCOLOR:
      g_value_set_uint (value, overlay->facenet_fontcolor);
      break;
    case PROP_FACENET_FONTSIZE:
      g_value_set_int (value, overlay->facenet_fontsize);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
release_lists (GstAmlOverlay *overlay) {
  cleanup_nn_list (overlay);
  cleanup_facenet_list (overlay, 0);
}

static gboolean
gst_aml_overlay_open (GstAmlOverlay * overlay)
{
  overlay->framenum = 0;
  g_mutex_init (&overlay->nn_list_mutex);
  list_init (&overlay->nn_list);
  g_mutex_init (&overlay->facenet_list_mutex);
  list_init (&overlay->facenet_list);
  return TRUE;
}

#define DESTROY_FONT(f) \
  do { \
    if (f) { \
      overlay_destroy_font (f); \
      f = NULL; \
    } \
  } while(0)

#define DESTROY_SURFACE(s) \
  do { \
    if (s) { \
      overlay_destroy_surface (s); \
      s = NULL; \
    } \
  } while(0)

#define FREE_STRING(s) \
  do { \
    if (s) { \
      g_free (s); \
      s = NULL; \
    } \
  } while(0)

static gboolean
gst_aml_overlay_close (GstAmlOverlay * overlay)
{
  DESTROY_FONT (overlay->clock_font);
  overlay->pts_font = NULL;

  DESTROY_SURFACE (overlay->clock_surface);

  DESTROY_SURFACE (overlay->pts_surface);

  DESTROY_FONT (overlay->watermark_font);
  DESTROY_SURFACE (overlay->watermark_text_surface);
  DESTROY_SURFACE (overlay->watermark_img_surface);

  DESTROY_FONT (overlay->facenet_font);

  DESTROY_SURFACE (overlay->facenet_text_surface);

  overlay_deinit();

  FREE_STRING (overlay->fontfile);
  FREE_STRING (overlay->watermark_text);
  FREE_STRING (overlay->watermark_fontfile);
  FREE_STRING (overlay->watermark_img);
  FREE_STRING (overlay->facenet_fontfile);

  release_lists (overlay);
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

static float
change_precision (float f) {
  float pf = (float)(((int)(f*100))/100.0);
  if (pf < 0.0) pf = 0.0;
  if (pf > 1.0) pf = 1.0;
  return pf;
}

static void
fix_rect_pos (struct RelativePos *pt) {
  pt->x0 = change_precision (pt->x0 - 0.01);
  pt->y0 = change_precision (pt->y0 - 0.01);
  pt->x1 = change_precision (pt->x1 + 0.01);
  pt->y1 = change_precision (pt->y1 + 0.01);
}

static void
cleanup_nn_list (GstAmlOverlay *overlay) {
  struct listnode *pos, *q;
  if (!list_empty (&overlay->nn_list)) {
    list_for_each_safe (pos, q, &overlay->nn_list) {
      struct NNResult *im =
        list_entry (pos, struct NNResult, list);
      list_remove (pos);
      if (im->pt) g_free (im->pt);
      g_free (im);
    }
  }
}

static void
cleanup_facenet_list (GstAmlOverlay *overlay, guint64 frameidx) {
  struct listnode *pos, *q;
  if (!list_empty (&overlay->facenet_list)) {
    list_for_each_safe (pos, q, &overlay->facenet_list) {
      struct FaceNetResult *im =
        list_entry (pos, struct FaceNetResult, list);
      if (frameidx < im->frameidx ||
          frameidx - im->frameidx > 1) {
        list_remove (pos);
        if (im->data->info) g_free (im->data->info);
        if (im->data) g_free (im->data);
        g_free (im);
      }
    }
  }
}

static gboolean
gst_aml_overlay_event (GstBaseTransform * base, GstEvent *event)
{
  GstAmlOverlay *overlay = GST_AMLOVERLAY (base);
  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CUSTOM_DOWNSTREAM_OOB:
      {
        const GstStructure *resst = gst_event_get_structure (event);
        if (gst_structure_has_name (resst, "facenet-detection")) {
          GstMapInfo info;
          const GValue *idx = gst_structure_get_value (resst, "idx");
          const GValue *faceinfo = gst_structure_get_value (resst, "faceinfo");
          guint64 frameidx = g_value_get_uint64 (idx);
          GstBuffer *faceinfobuf = gst_value_get_buffer (faceinfo);
          struct FaceNetEventBuffer *fr =
            (struct FaceNetEventBuffer *)g_malloc (sizeof(struct FaceNetEventBuffer));

          if (gst_buffer_map (faceinfobuf, &info, GST_MAP_READ)) {
            size_t faceinfo_size = info.size - sizeof(struct RelativePos);
            fr->info = faceinfo_size > 0 ? (char *) g_malloc (faceinfo_size) : NULL;
            g_memmove (&fr->pos, info.data, sizeof(struct RelativePos));
            fix_rect_pos (&fr->pos);
            if (fr->info) {
              g_memmove (fr->info, info.data + sizeof(struct RelativePos), faceinfo_size);
            }
            gst_buffer_unmap (faceinfobuf, &info);
          } else {
            if (fr) g_free (fr);
            fr = NULL;
          }

          if (fr) {
            struct FaceNetResult *new_result =
              (struct FaceNetResult *)g_malloc (sizeof (struct FaceNetResult));
            new_result->frameidx = frameidx;
            new_result->data = fr;
            list_init (&new_result->list);
            g_mutex_lock (&overlay->facenet_list_mutex);
            // cleanup the old entries
            cleanup_facenet_list (overlay, frameidx);
            // add newest entries
            list_add_tail (&overlay->facenet_list, &new_result->list);
            g_mutex_unlock (&overlay->facenet_list_mutex);
          }

          gst_event_unref (event);
          return FALSE;
        }
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
            for (gint i = 0; i < detect_num; i++) {
              struct RelativePos *p = &pt[i];
              fix_rect_pos (p);
            }
            struct NNResult *new_result = (struct NNResult *)g_malloc (sizeof(struct NNResult));
            new_result->detect_num = detect_num;
            new_result->frameidx = frameidx;
            new_result->pt = pt;
            list_init (&new_result->list);
            g_mutex_lock (&overlay->nn_list_mutex);
            // cleanup the old entries
            cleanup_nn_list (overlay);
            // add newest entries
            list_add_tail (&overlay->nn_list, &new_result->list);
            g_mutex_unlock (&overlay->nn_list_mutex);
          }

          gst_event_unref (event);
          return FALSE;
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

  overlay->framenum ++;

  GstVideoInfo *info = &overlay->info;
  GstMapInfo outbuf_info;
  if (gst_buffer_map (outbuf, &outbuf_info, GST_MAP_READ | GST_MAP_WRITE)) {
    overlay_init();
    overlay_create_inputbuffer (outbuf_info.data, info->width, info->height);
    if (overlay->draw_clock || overlay->draw_pts) {
      if (overlay->clock_font == NULL ||
          overlay->font_changed) {
        overlay->font_changed = FALSE;
        if (overlay->clock_font) {
          overlay_destroy_font (overlay->clock_font);
        }
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
      if (overlay->watermark_font == NULL ||
          overlay->watermark_text_font_changed) {
        if (overlay->watermark_font) {
          overlay_destroy_font (overlay->watermark_font);
        }
        overlay->watermark_font = overlay_create_font (overlay->watermark_fontfile,
            overlay->watermark_fontsize, 0);
      }
      if (overlay->watermark_text_surface == NULL ||
          overlay->watermark_text_changed ||
          overlay->watermark_text_font_changed) {
        if (overlay->watermark_text_surface) {
          overlay_destroy_surface (overlay->watermark_text_surface);
        }
        overlay->watermark_text_surface = overlay_create_text_surface (overlay->watermark_text,
            overlay->watermark_font, overlay->watermark_fontcolor, 0);
      }
      overlay->watermark_text_font_changed = FALSE;
      overlay->watermark_text_changed = FALSE;
      overlay_draw_surface (overlay->watermark_text_surface,
          overlay->watermark_text_xpos, overlay->watermark_text_ypos);
    }

    if (strlen (overlay->watermark_img) > 0) {
      if (overlay->watermark_img_surface == NULL ||
          overlay->watermark_img_changed) {
        overlay->watermark_img_changed = FALSE;
        if (overlay->watermark_img_surface) {
          overlay_destroy_surface (overlay->watermark_img_surface);
        }
        overlay->watermark_img_surface = overlay_create_image_surface (overlay->watermark_img,
            overlay->watermark_img_width, overlay->watermark_img_height);
      }
      overlay_draw_surface (overlay->watermark_img_surface,
          overlay->watermark_img_xpos, overlay->watermark_img_ypos);
    }
    if (overlay->nnrect_show) {
      g_mutex_lock (&overlay->nn_list_mutex);
      if (!list_empty (&overlay->nn_list)) {
        struct listnode *pos;
        list_for_each (pos, &overlay->nn_list) {
          struct NNResult *im =
            list_entry (pos, struct NNResult, list);
          if (im->frameidx >= overlay->framenum ||
              overlay->framenum - im->frameidx < DETECT_DELAY_FRAMES) {
            for (gint i = 0; i < im->detect_num; i++) {
              struct RelativePos *pt = &im->pt[i];
              overlay_draw_rect(
                  (gint)(pt->x0 * info->width),
                  (gint)(pt->y0 * info->height),
                  (gint)((pt->x1 - pt->x0) * info->width),
                  (gint)((pt->y1 - pt->y0) * info->height),
                  5, overlay->nn_rectcolor);
            }
          }
        }
      }
      g_mutex_unlock (&overlay->nn_list_mutex);
    }
    if (overlay->facenet_show) {
      g_mutex_lock (&overlay->facenet_list_mutex);
      if (!list_empty (&overlay->facenet_list)) {
        // prepare facenet font
        if (overlay->facenet_font == NULL ||
            overlay->facenet_font_changed) {
          overlay->facenet_font_changed = FALSE;
          if (overlay->facenet_font) {
            overlay_destroy_font (overlay->facenet_font);
          }
          overlay->facenet_font = overlay_create_font (overlay->facenet_fontfile,
              overlay->facenet_fontsize, 0);
        }
        struct listnode *pos;
        list_for_each (pos, &overlay->facenet_list) {
          struct FaceNetResult *im =
            list_entry (pos, struct FaceNetResult, list);
          if (im->frameidx >= overlay->framenum ||
              overlay->framenum - im->frameidx < DETECT_DELAY_FRAMES) {
            struct RelativePos *pt = &im->data->pos;
            overlay_draw_rect(
                (gint)(pt->x0 * info->width),
                (gint)(pt->y0 * info->height),
                (gint)((pt->x1 - pt->x0) * info->width),
                (gint)((pt->y1 - pt->y0) * info->height),
                5, overlay->facenet_rectcolor);


            if (im->data->info && strlen(im->data->info) > 0) {
              gchar* txt = g_strdup (im->data->info);
              gint x = (gint)(pt->x0 * info->width) + overlay->facenet_fontsize * 2;
              gint y = (gint)(pt->y0 * info->height) + overlay->facenet_fontsize;
              overlay->facenet_text_surface =
                overlay_create_text_surface (txt, overlay->facenet_font, overlay->facenet_fontcolor, 0);
              g_free(txt);
              overlay_draw_surface (overlay->facenet_text_surface, x, y);
              overlay_destroy_surface (overlay->facenet_text_surface);
              overlay->facenet_text_surface = NULL;
            }
          }
        }
      }
      g_mutex_unlock (&overlay->facenet_list_mutex);
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
