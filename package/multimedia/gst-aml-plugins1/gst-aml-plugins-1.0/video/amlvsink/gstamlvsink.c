/* GStreamer */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "gstamlvsink.h"

GST_DEBUG_CATEGORY_STATIC (gst_aml_vsink_debug);
#define GST_CAT_DEFAULT gst_aml_vsink_debug
#define VERSION	"1.1"

enum
{
  ARG_0,
};

#if 0
static void gst_aml_vsink_get_times (GstBaseSink * basesink,
    GstBuffer * buffer, GstClockTime * start, GstClockTime * end);
#endif

static GstFlowReturn gst_aml_vsink_show_frame (GstVideoSink * videosink, GstBuffer * buff);

static gboolean gst_aml_vsink_start (GstBaseSink * bsink);
static gboolean gst_aml_vsink_stop (GstBaseSink * bsink);

static GstCaps *gst_aml_vsink_getcaps (GstBaseSink * bsink, GstCaps * filter);
static gboolean gst_aml_vsink_setcaps (GstBaseSink * bsink, GstCaps * caps);

static void gst_aml_vsink_finalize (GObject * object);
static void gst_aml_vsink_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_aml_vsink_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static GstStateChangeReturn gst_aml_vsink_change_state (GstElement * element, GstStateChange transition);

#define VIDEO_CAPS "{ xRGB }"

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
    	    "format = (string) { xRGB }, "
    	    "width = " GST_VIDEO_SIZE_RANGE ", "
    	    "height = " GST_VIDEO_SIZE_RANGE ", "
    	    "framerate = " GST_VIDEO_FPS_RANGE )
    );

#define parent_class gst_aml_vsink_parent_class
G_DEFINE_TYPE (GstAmlVsink, gst_aml_vsink, GST_TYPE_VIDEO_SINK);

static void
gst_aml_vsink_class_init (GstAmlVsinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *basesink_class;
  GstVideoSinkClass *videosink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  basesink_class = (GstBaseSinkClass *) klass;
  videosink_class = (GstVideoSinkClass *) klass;

  gobject_class->set_property = gst_aml_vsink_set_property;
  gobject_class->get_property = gst_aml_vsink_get_property;
  gobject_class->finalize = gst_aml_vsink_finalize;

  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_aml_vsink_change_state);

  basesink_class->set_caps = GST_DEBUG_FUNCPTR (gst_aml_vsink_setcaps);
  basesink_class->get_caps = GST_DEBUG_FUNCPTR (gst_aml_vsink_getcaps);
  basesink_class->start = GST_DEBUG_FUNCPTR (gst_aml_vsink_start);
  basesink_class->stop = GST_DEBUG_FUNCPTR (gst_aml_vsink_stop);

  videosink_class->show_frame = GST_DEBUG_FUNCPTR (gst_aml_vsink_show_frame);

  gst_element_class_set_static_metadata (gstelement_class,
		  "Amlogic Fake Video Sink",
		  "Sink/Video",
		  "Amlogic fake videosink",
		  "mm@amlogic.com>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));
}

static void
gst_aml_vsink_init (GstAmlVsink * amlvsink)
{
	GstVideoSink *bsink;
	bsink = GST_VIDEO_SINK (amlvsink);
     gst_base_sink_set_sync (GST_BASE_SINK (amlvsink), FALSE);
     gst_base_sink_set_async_enabled (GST_BASE_SINK(amlvsink), FALSE);
}


static void
gst_aml_vsink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmlVsink *amlvsink;

  amlvsink = GST_AMLVSINK (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
gst_aml_vsink_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstAmlVsink *amlvsink;

  amlvsink = GST_AMLVSINK (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_aml_vsink_finalize (GObject * object)
{
  GstAmlVsink *amlvsink = GST_AMLVSINK (object);
  GST_DEBUG_OBJECT(amlvsink, "%s", __FUNCTION__);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GstStateChangeReturn
gst_aml_vsink_change_state (GstElement * element, GstStateChange transition)
{
    GstAmlVsink *amlvsink= GST_AMLVSINK (element);
    GstStateChangeReturn result;
    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:
			 GST_ERROR("%s,%d\n",__FUNCTION__,__LINE__);
                //prepared function
            break;
        case GST_STATE_CHANGE_READY_TO_PAUSED:
			 GST_ERROR("%s,%d\n",__FUNCTION__,__LINE__);
            break;
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
			 GST_ERROR("%s,%d\n",__FUNCTION__,__LINE__);
            break;
        default:
            break;
    }

    result = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  
    switch (transition) {
	  case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
	  	 GST_ERROR("%s,%d\n",__FUNCTION__,__LINE__);

	      break;
        case GST_STATE_CHANGE_PAUSED_TO_READY: 
			 GST_ERROR("%s,%d\n",__FUNCTION__,__LINE__);
            break;
        case GST_STATE_CHANGE_READY_TO_NULL:  
			 GST_ERROR("%s,%d\n",__FUNCTION__,__LINE__);
            break;
        default:
            break;
    }
	GST_ERROR("%s,%d,result=%d\n",__FUNCTION__,__LINE__,result);
  return result;
}

static gboolean
gst_aml_vsink_setcaps (GstBaseSink * bsink, GstCaps * vscapslist)
{
  GstAmlVsink *amlvsink;
  GstStructure *structure;
  const GValue *fps;

  amlvsink = GST_AMLVSINK (bsink);
  GST_DEBUG_OBJECT(amlvsink, "%s", __FUNCTION__);
  structure = gst_caps_get_structure (vscapslist, 0);
  return TRUE;
}

static GstCaps *
gst_aml_vsink_getcaps (GstBaseSink * bsink, GstCaps * filter)
{
  GstAmlVsink *amlvsink;
  GstVideoFormat format;
  GstCaps *caps;

  amlvsink = GST_AMLVSINK (bsink);
  GST_DEBUG_OBJECT(amlvsink, "%s", __FUNCTION__);
  caps = gst_static_pad_template_get_caps (&sink_template);


  format = GST_VIDEO_FORMAT_xRGB;
  caps = gst_caps_make_writable (caps);
  gst_caps_set_simple (caps, "format", G_TYPE_STRING,
      gst_video_format_to_string (format), NULL);

done:

  if (filter != NULL) {
    GstCaps *icaps;

    icaps = gst_caps_intersect (caps, filter);
    gst_caps_unref (caps);
    caps = icaps;
  }
  return caps;
}

static gboolean
gst_aml_vsink_start (GstBaseSink * bsink)
{
  GstAmlVsink *amlvsink;

  amlvsink = GST_AMLVSINK (bsink);
  GST_DEBUG_OBJECT(amlvsink, "%s", __FUNCTION__);
  return TRUE;
}

static gboolean
gst_aml_vsink_stop (GstBaseSink * bsink)
{
  GstAmlVsink *amlvsink;

  amlvsink = GST_AMLVSINK (bsink);
  GST_DEBUG_OBJECT(amlvsink, "%s", __FUNCTION__);
  return TRUE;
}

static GstFlowReturn
gst_aml_vsink_show_frame (GstVideoSink * videosink, GstBuffer * buf)
{

  GstAmlVsink *amlvsink;
  amlvsink = GST_AMLVSINK (videosink);
  GST_DEBUG_OBJECT(amlvsink, "%s %llu", __FUNCTION__, GST_BUFFER_TIMESTAMP (buf));
  return GST_FLOW_OK;
}

static gboolean
amlvsink_init (GstPlugin * plugin)
{
	GST_DEBUG_CATEGORY_INIT(gst_aml_vsink_debug, "amlvsink", 0, "Amlogic Fake Video Sink");
	return gst_element_register(plugin, "amlvsink", GST_RANK_SECONDARY, GST_TYPE_AMLVSINK);
}


#ifndef PACKAGE
#define PACKAGE "gst-plugins-amlogic"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amlvsink,
    "Amlogic Fake Video Sink",
    amlvsink_init,
	VERSION,
	"LGPL",
	"Amlogic",
	"http://amlogic.com/"
)
