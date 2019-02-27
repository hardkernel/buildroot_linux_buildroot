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

#ifndef __GST_AMLIMGCAP_H__
#define __GST_AMLIMGCAP_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_AMLIMGCAP \
  (gst_aml_imgcap_get_type())
#define GST_AMLIMGCAP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AMLIMGCAP,GstAmlImageCap))
#define GST_AMLIMGCAP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AMLIMGCAP,GstAmlImageCapClass))
#define GST_IS_AMLIMGCAP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AMLIMGCAP))
#define GST_IS_AMLIMGCAP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AMLIMGCAP))

typedef struct _GstAmlImageCap      GstAmlImageCap;
typedef struct _GstAmlImageCapClass GstAmlImageCapClass;

struct _GstAmlImageCap {
  GstBaseTransform element;

  /*< private >*/
  gchar *location;
  gboolean b_capreq;

  /* properties */
  gint quality;

  GstVideoInfo info;
  gboolean is_info_set;
};

struct _GstAmlImageCapClass {
  GstBaseTransformClass parent_class;
};

GType gst_aml_imgcap_get_type (void);

G_END_DECLS

#endif /* __GST_AMLIMGCAP_H__ */
