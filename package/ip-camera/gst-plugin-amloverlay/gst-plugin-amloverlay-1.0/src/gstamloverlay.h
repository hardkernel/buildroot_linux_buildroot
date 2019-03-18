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

#ifndef __GST_AMLOVERLAY_H__
#define __GST_AMLOVERLAY_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_AMLOVERLAY \
  (gst_aml_overlay_get_type())
#define GST_AMLOVERLAY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AMLOVERLAY,GstAmlOverlay))
#define GST_AMLOVERLAY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AMLOVERLAY,GstAmlOverlayClass))
#define GST_IS_AMLOVERLAY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AMLOVERLAY))
#define GST_IS_AMLOVERLAY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AMLOVERLAY))

typedef struct _GstAmlOverlay      GstAmlOverlay;
typedef struct _GstAmlOverlayClass GstAmlOverlayClass;

/**
 * GstBaseTextOverlayVAlign:
 * @GST_BASE_TEXT_OVERLAY_VALIGN_BASELINE: draw text on the baseline
 * @GST_BASE_TEXT_OVERLAY_VALIGN_BOTTOM: draw text on the bottom
 * @GST_BASE_TEXT_OVERLAY_VALIGN_TOP: draw text on top
 * @GST_BASE_TEXT_OVERLAY_VALIGN_POS: draw text according to the #GstBaseTextOverlay:ypos property
 * @GST_BASE_TEXT_OVERLAY_VALIGN_CENTER: draw text vertically centered
 *
 * Vertical alignment of the text.
 */
typedef enum {
    GST_AML_TEXT_OVERLAY_POS_TOP_LEFT,
    GST_AML_TEXT_OVERLAY_POS_TOP_MID,
    GST_AML_TEXT_OVERLAY_POS_TOP_RIGHT,
    GST_AML_TEXT_OVERLAY_POS_MID_LEFT,
    GST_AML_TEXT_OVERLAY_POS_MID_RIGHT,
    GST_AML_TEXT_OVERLAY_POS_CENTER,
    GST_AML_TEXT_OVERLAY_POS_BOTTOM_LEFT,
    GST_AML_TEXT_OVERLAY_POS_BOTTOM_MID,
    GST_AML_TEXT_OVERLAY_POS_BOTTOM_RIGHT,
} GstAmlTextOverlayPOS;

struct _GstAmlOverlay {
  GstBaseTransform element;

  /*< private >*/
  void *clock_font;
  void *clock_surface;
  void *pts_font;
  void *pts_surface;
  void *watermark_font;
  void *watermark_text_surface;
  void *watermark_img_surface;

  /* properties */
  gboolean draw_clock;
  gboolean draw_pts;
  gboolean draw_outline;
  gboolean disable_facerect;
  gchar* fontfile;
  gint fontsize;
  guint fontcolor;
  guint bgcolor;
  GstAmlTextOverlayPOS clock_pos;
  GstAmlTextOverlayPOS pts_pos;
  gchar* watermark_text;
  gchar* watermark_fontfile;
  guint watermark_fontcolor;
  gint watermark_fontsize;
  gint watermark_text_xpos;
  gint watermark_text_ypos;
  gchar* watermark_img;
  gint watermark_img_xpos;
  gint watermark_img_ypos;
  gint watermark_img_width;
  gint watermark_img_height;
  guint facerect_color;
  guint facetext_color;
  gint facetext_size;

  GstVideoInfo info;
  gboolean is_info_set;

  gboolean font_changed;
  gboolean watermark_text_font_changed;
  gboolean watermark_text_changed;
  gboolean watermark_img_changed;
};

struct _GstAmlOverlayClass {
  GstBaseTransformClass parent_class;
};

GType gst_aml_overlay_get_type (void);

G_END_DECLS

#endif /* __GST_AMLOVERLAY_H__ */
