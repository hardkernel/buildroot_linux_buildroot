
#include "gstamlvdec.h"
#include "amlvdec_prop.h"

static void amlInstallPropTrickRate(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_double ("trickrate", "TrickRate", "trick-play client support",
          -1.0, 2.0, 1.0, G_PARAM_READWRITE));
}

static void amlInstallPropInterlaced(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_boolean ("interlaced", "Interlaced", "property of the stream",
          FALSE, G_PARAM_READABLE));
}
static void amlInstallPropDecodeHandle(GObjectClass *oclass,
    guint property_id)
{
    GObjectClass*gobject_class = (GObjectClass *) oclass;
    g_object_class_install_property (gobject_class, property_id, g_param_spec_pointer ("decodehandle", "DecodeHandle", "Decoder identifier which can be used to retrieve user data from decoder",
          G_PARAM_READWRITE));
}

static void amlInstallPropSystemtimetoposition(GObjectClass *oclass,    
	guint property_id)
{    
		GObjectClass *gobject_class = (GObjectClass *) oclass;    
		g_object_class_install_property (gobject_class, property_id, g_param_spec_int64("position", NULL, NULL,0, G_MAXINT64, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static int amlGetPropTrickRate(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVdec *amlvdec = GST_AMLVDEC (object); 
    g_value_set_double (value, amlvdec->trickRate);
    return 0;
}
static int amlGetPropInterlaced(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVdec *amlvdec = GST_AMLVDEC (object); 
    gboolean interlaced = FALSE;
    char format[64] = {0};
    if(get_sysfs_str("/sys/class/video/frame_format", format, sizeof(format))){
        return -1;
    }
    if(!strncmp(format, "interlace", strlen("interlace"))){
        interlaced = TRUE;
    }
    g_value_set_boolean (value, interlaced);
    return 0;
}
static int amlGetPropDecodeHandle(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVdec *amlvdec = GST_AMLVDEC (object); 
    g_value_set_pointer (value, (gpointer)amlvdec->decHandle);
    return 0;
}

static int amlGetPropSystemtimetoposition(GObject * object, guint prop_id,    
		const GValue * value, GParamSpec * pspec)
{		
		GstAmlVdec *amlvdec = GST_AMLVDEC (object);  
		gint64 pcrscr = 0;		
		gint64 position = 0;		
		if(amlvdec->bpass) {			
			/*assume query first pcrscr is the basepcr*/			
			amlvdec->basepcr = codec_get_pcrscr(amlvdec->pcodec);			
			amlvdec->bpass = FALSE;		
		}		
		else {			
			pcrscr = codec_get_pcrscr(amlvdec->pcodec);	
			position = (pcrscr - amlvdec->basepcr) * (1000000 / 10) / 9LL;;			
				
		}	 
		g_value_set_int64 (value, position);	
		return 0;
}

static int amlSetPropTrickRate(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVdec *amlvdec = GST_AMLVDEC (object); 
    gdouble trickRate = g_value_get_double(value);

    if(amlvdec_forward_process(amlvdec, 0, trickRate, GST_FORMAT_DEFAULT, 0, 0, 0)){
        amlvdec->trickRate = trickRate;
    }

    return 0;
}

static int amlSetPropDecodeHandle(GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlVdec *amlvdec = GST_AMLVDEC (object);  
    amlvdec->decHandle = g_value_get_pointer (value);
    return 0;
}

static const AmlPropType amlvdec_prop_pool[] = {
    {PROP_TRICK_RATE,   amlInstallPropTrickRate,          amlGetPropTrickRate,         amlSetPropTrickRate       },
    {PROP_INTERLACED,   amlInstallPropInterlaced,         amlGetPropInterlaced,       NULL      },   
    {PROP_DEC_HDL,       amlInstallPropDecodeHandle,   amlGetPropDecodeHandle, amlSetPropDecodeHandle},
    {PROP_PCRTOSYSTEMTIME, amlInstallPropSystemtimetoposition, amlGetPropSystemtimetoposition, NULL},
    {-1,                          NULL,                                         NULL,                                  NULL},
};

AmlPropType *aml_get_vdec_prop_interface()
{
    return &amlvdec_prop_pool[0];
}


