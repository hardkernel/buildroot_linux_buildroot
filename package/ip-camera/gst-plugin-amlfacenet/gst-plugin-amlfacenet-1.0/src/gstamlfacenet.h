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

#ifndef __GST_AMLFACENET_H__
#define __GST_AMLFACENET_H__

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>
#include <nn_detect.h>
#include "list.h"

G_BEGIN_DECLS

#define GST_TYPE_AMLFACENET \
  (gst_aml_facenet_get_type())
#define GST_AMLFACENET(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AMLFACENET,GstAmlFacenet))
#define GST_AMLFACENET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AMLFACENET,GstAmlFacenetClass))
#define GST_IS_AMLFACENET(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AMLFACENET))
#define GST_IS_AMLFACENET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AMLFACENET))

typedef struct _GstAmlFacenetThreadInfo GstAmlFacenetThreadInfo;
typedef struct _GstAmlFacenet      GstAmlFacenet;
typedef struct _GstAmlFacenetClass GstAmlFacenetClass;

struct _GstAmlFacenetThreadInfo {
  GThread *thread;
  GMutex mutex;
  GCond cond;
};

struct _GstAmlFacenet {
  GstBaseSink element;
  /* properties */
  gchar *string_format;
  gchar *dbfile;
  gfloat threshold;
  gboolean b_store_face;

  /*< private >*/
  det_model_type model_type;
  gint model_width;
  gint model_height;
  gint model_channel;

  GstAmlFacenetThreadInfo procinfo_grabface;
  GstAmlFacenetThreadInfo procinfo_facenet;
  GstAmlFacenetThreadInfo procinfo_facenet_result;
  GstAmlFacenetThreadInfo procinfo_facenet_db;;
  gboolean _running;

  GstVideoInfo info;
  gboolean is_info_set;

  GstPad *src_srcpad;

  void *db_handle;

  struct listnode face_list;
  struct listnode facenet_ilist;
  struct listnode facenet_rlist;

};

struct _GstAmlFacenetClass {
  GstBaseSinkClass parent_class;
};

GType gst_aml_facenet_get_type (void);

G_END_DECLS

#endif /* __GST_AMLFACENET_H__ */
