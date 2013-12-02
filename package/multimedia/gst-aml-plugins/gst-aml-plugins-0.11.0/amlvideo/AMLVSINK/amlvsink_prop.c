
#include "gstamlvsink.h"
#include "amlvsink_prop.h"

/*video sink property*/
static void amlInstallPropSilent(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));
}
static void amlInstallPropAsync(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_boolean ("async", "Async", "Video synchronized with audio",
          TRUE, G_PARAM_READWRITE));
}
static void amlInstallPropTvMode(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_int ("tvmode", "TvMode", "Define the television mode",
          -10, 10, 0, G_PARAM_READWRITE));
}
static void amlInstallPropPlane(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_int ("plane", "Plane", "Define the Pixel Plane to be used",
          -2000, 2000, 0, G_PARAM_READWRITE));
}
static void amlInstallPropRectangle(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_int ("rectangle", "Rectangle", "The destination rectangle",
          -2000, 2000, 0, G_PARAM_READWRITE));
}
static void amlInstallPropFlushRepeatFrame(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_boolean ("flushRepeatFrame", "FlushRepeatFrame", "Keep displaying the last frame rather than a black one whilst flushing",
          FALSE, G_PARAM_READWRITE));
}
static void amlInstallPropCurrentPTS(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_int64 ("currentPTS", "CurrentPTS", "Display current PTS",
          0, 0x1FFFFFFF, 0, G_PARAM_READABLE));
}
static void amlInstallPropInterFrameDelay(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_boolean ("frameDelay", "FrameDelay", "Enables fixed frame rate mode",
          FALSE, G_PARAM_READWRITE));
}
static void amlInstallPropSlowModeRate(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_int ("slowModeRate", "SlowModeRate", "slow mode rate in normalised form",
          -20000, 20000, 0, G_PARAM_READWRITE));
}

static void amlInstallPropContentFrameRate(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_int ("contentFrameRate", "ContentFrameRate", "",
          0, 100, 30, G_PARAM_READWRITE));
}

static void amlInstallPropStepFrame(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_int ("stepFrame", "StepFrame", "get content frame rate",
          -2, 2, 1, G_PARAM_READWRITE));
}

static void amlInstallPropMute(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_int ("mute", "Mute", "",
          -1, 1, 0, G_PARAM_READWRITE));
}

static int amlGetPropSilent(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_boolean (value, filter->bSilent);
    return 0;
}

static int amlGetPropAsync(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_boolean (value, filter->bAsync);
    
    return 0;
}

static int amlGetPropTvMode(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_int (value, filter->tvMode);
    return 0;
}
static int amlGetPropPlane(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_int (value, filter->plane);
    return 0;
}
static int amlGetPropRectangle(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_int (value, filter->rectangle);
    return 0;
}
static int amlGetPropFlushRepeatFrame(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_boolean (value, filter->bFlsRptFrm);
    g_value_set_boolean (value, get_black_policy());
    return 0;
}
static int amlGetPropCurrentPTS(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gint64 currentPTS = 0;
    int pts = get_sysfs_int("/sys/class/tsync/pts_video");
    //currentPTS = (gint64)codec_get_vpts(vpcodec);
    currentPTS = (gint64)pts;
    g_print("amlGetPropCurrentPTS pts=%d currentPTS=%lld\n", pts, currentPTS);
    g_value_set_int64 (value, currentPTS);
    return 0;
}
static int amlGetPropInterFrameDelay(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_boolean (value, amlsink->bInterFrmDly);
    return 0;
}
static int amlGetPropSlowModeRate(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_int (value, filter->slowModeRate);
    return 0;
}
static int amlGetPropContentFrameRate(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_int (value, filter->contentFrmRate);
    return 0;
}
static int amlGetPropStepFrame(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_int (value, filter->stepFrm);
    return 0;
}
static int amlGetPropMute(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //g_value_set_int (value, filter->mute);
    return 0;
}

static int amlSetPropSilent(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object);  
    gboolean bSilent = g_value_get_boolean (value);
    return 0;
}

static int amlSetPropAsync(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object);
    gboolean bAsync = FALSE;
    bAsync = g_value_get_boolean (value);
    gst_base_sink_set_async_enabled (GST_BASE_SINK(amlsink), bAsync);
    return 0;
}

static int amlSetPropTvMode(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gint tvMode = g_value_get_int(value);
    return 0;
}
static int amlSetPropPlane(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gint plane = g_value_get_int(value);
    return 0;
}
static int amlSetPropRectangle(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gint rectangle = g_value_get_int(value);
    return 0;
}
static int amlSetPropFlushRepeatFrame(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    //amlsink->bFlsRptFrm = g_value_get_boolean(value);
    set_black_policy (g_value_get_boolean (value));
    return 0;
}

static int amlSetPropInterFrameDelay(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gboolean bInterFrmDly = g_value_get_boolean(value);
    return 0;
}
static int amlSetPropSlowModeRate(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gint slowModeRate = g_value_get_int(value);
    return 0;
}
static int amlSetPropContentFrameRate(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gint contentFrmRate = g_value_get_int(value);
    return 0;
}
static int amlSetPropStepFrame(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gint stepFrm = g_value_get_int(value);
    return 0;
}
static int amlSetPropMute(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVsink *amlsink = GST_AMLVSINK (object); 
    gint mute = g_value_get_int(value);
    return 0;
}

/*video render property install/get/set function pool*/
static const AmlPropType amlvsink_prop_pool[] = {
    {PROP_SILENT,                amlInstallPropSilent,                      amlGetPropSilent,                       amlSetPropSilent},
    {PROP_ASYNC ,                amlInstallPropAsync,                      amlGetPropAsync,                       amlSetPropAsync},
    {PROP_TVMODE,              amlInstallPropTvMode,                    amlGetPropTvMode,                    amlSetPropTvMode},
    {PROP_PLANE,                 amlInstallPropPlane,                       amlGetPropPlane,                       amlSetPropPlane},
    {PROP_RECTANGLE,         amlInstallPropRectangle,                 amlGetPropRectangle,                amlSetPropRectangle},
    {PROP_FLSH_RPT_FRM,    amlInstallPropFlushRepeatFrame,    amlGetPropFlushRepeatFrame,   amlSetPropFlushRepeatFrame},
    {PROP_CURT_PTS,           amlInstallPropCurrentPTS,               amlGetPropCurrentPTS,              NULL},
    {PROP_INTER_FRM_DELY, amlInstallPropInterFrameDelay,      amlGetPropInterFrameDelay,      amlSetPropInterFrameDelay},
    {PROP_SLOW_FRM_RATE, amlInstallPropSlowModeRate,          amlGetPropSlowModeRate,         amlSetPropSlowModeRate},
    {PROP_CONT_FRM_RATE,  amlInstallPropContentFrameRate,   amlGetPropContentFrameRate,   NULL},
    {PROP_STEP_FRM,            amlInstallPropStepFrame,               amlGetPropStepFrame,                amlSetPropStepFrame},
    {PROP_MUTE,                  amlInstallPropMute,                         amlGetPropMute,                         amlSetPropMute},
    {-1,                                NULL,                                                NULL,                                           NULL},
};

AmlPropType *aml_get_vsink_prop_interface()
{
    return &amlvsink_prop_pool[0];
}


