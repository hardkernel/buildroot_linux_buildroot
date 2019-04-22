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

#ifndef __GST_AMLNN_H__
#define __GST_AMLNN_H__

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>
#include <nn_detect.h>

G_BEGIN_DECLS

#define GST_TYPE_AMLNN \
  (gst_aml_nn_get_type())
#define GST_AMLNN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AMLNN,GstAmlNN))
#define GST_AMLNN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AMLNN,GstAmlNNClass))
#define GST_IS_AMLNN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AMLNN))
#define GST_IS_AMLNN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AMLNN))

typedef struct _GstAmlNN      GstAmlNN;
typedef struct _GstAmlNNClass GstAmlNNClass;

struct _GstAmlNN {
  GstBaseTransform element;
  /* properties */
  det_model_type model_type;

  /*< private >*/
  det_model_type model_type_old;
  gint model_width;
  gint model_height;
  gint model_channel;
  GThread *_thread;
  GMutex _mutex;
  GCond _cond;
  gboolean _running;

  GstVideoInfo info;
  gboolean is_info_set;
  gboolean b_model_set;

  gint framerate;
  guint64 framenum;
  guint64 next_detect_framenum;
};

struct _GstAmlNNClass {
  GstBaseTransformClass parent_class;
};

GType gst_aml_nn_get_type (void);

G_END_DECLS

#endif /* __GST_AMLNN_H__ */
