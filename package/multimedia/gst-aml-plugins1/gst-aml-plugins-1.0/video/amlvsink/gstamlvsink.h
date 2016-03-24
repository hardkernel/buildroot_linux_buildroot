/* GStreamer */


#ifndef __GST_GSTAMLVSINK_H__
#define __GST_GSTAMLVSINK_H__

#include <gst/gst.h>
#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>


G_BEGIN_DECLS

#define GST_TYPE_AMLVSINK \
  (gst_aml_vsink_get_type())
#define GST_AMLVSINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AMLVSINK,GstAmlVsink))
#define GST_AMLVSINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AMLVSINK,GstAmlVsinkClass))
#define GST_AMLVSINK_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj),GST_TYPE_AMLVSINK,GstAmlVsinkClass))  
#define GST_IS_AMLVSINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AMLVSINK))
#define GST_IS_AMLVSINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AMLVSINK))

typedef struct _GstAmlVsink GstAmlVsink;
typedef struct _GstAmlVsinkClass GstAmlVsinkClass;

struct _GstAmlVsink {
  GstVideoSink videosink;
};

struct _GstAmlVsinkClass {
  GstVideoSinkClass videosink_class;
};

GType gst_aml_vsink_get_type(void);

G_END_DECLS

#endif /* __GST_GSTAMLVSINK_H__ */
