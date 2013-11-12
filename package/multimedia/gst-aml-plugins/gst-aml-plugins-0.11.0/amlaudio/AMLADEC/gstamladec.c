/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2013  <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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
 * SECTION:element-amladec
 *
 * FIXME:Describe amladec here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amladec ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstamladec.h"
#include  "gstamlaudioheader.h"
#include  "gstamlsysctl.h"
#include  "../../common/include/codec.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>

GST_DEBUG_CATEGORY_STATIC (gst_amladec_debug);
#define GST_CAT_DEFAULT gst_amladec_debug
#define  AML_DEBUG(...)   GST_INFO_OBJECT(amladec,__VA_ARGS__) 
//#define  AML_DEBUG   g_print
/* Filter signals and args */
enum
{
    /* FILL ME */
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_SILENT,
    PROP_SYNC,
    PROP_ASYNC
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );
GST_BOILERPLATE (GstAmlAdec, gst_amladec, GstBaseTransform, GST_TYPE_BASE_TRANSFORM);

static void gst_amladec_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_amladec_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static GstStateChangeReturn gst_amladec_change_state (GstElement *element, GstStateChange transition);
static GstFlowReturn gst_amladec_render (GstAmlAdec *amladec, GstBuffer *buffer);
static gboolean gst_amladec_set_caps (GstBaseTransform * base, GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_amladec_start(GstBaseTransform *trans);
static gboolean gst_amladec_stop(GstBaseTransform *trans);
static gboolean gst_amladec_sink_event(GstBaseTransform *trans, GstEvent *event);
static gboolean gst_amladec_src_query (GstPad    *pad, GstQuery  *query);
static void gst_amladec_before_transform (GstBaseTransform *trans, GstBuffer *buffer);
static GstFlowReturn gst_amladec_transform_ip (GstBaseTransform *trans, GstBuffer *buf);



/* GObject vmethod implementations */
static codec_para_t a_codec_para;
static codec_para_t *apcodec;

/* GObject vmethod implementations */

static void
gst_amladec_base_init (gpointer gclass)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);
    gst_element_class_set_details_simple(element_class,
    "amladec",
    "audio out dsp decoder ",
    "ts audio out plugin using dsp decoder",
    " <<aml@aml.org>>");
    gst_element_class_add_pad_template (element_class,
    gst_static_pad_template_get (&src_factory));
    gst_element_class_add_pad_template (element_class,
    gst_static_pad_template_get (&sink_factory));
}

/* initialize the amladec's class */
static void
gst_amladec_class_init (GstAmlAdecClass * klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;
    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;
    GstBaseTransformClass *basetransform_class = GST_BASE_TRANSFORM_CLASS (klass);
    gobject_class->set_property = gst_amladec_set_property;
    gobject_class->get_property = gst_amladec_get_property;
    gstelement_class->change_state = gst_amladec_change_state;

    g_object_class_install_property (gobject_class, PROP_SILENT, g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));
	
    basetransform_class->transform_ip = GST_DEBUG_FUNCPTR ( gst_amladec_transform_ip);
    basetransform_class->set_caps = GST_DEBUG_FUNCPTR ( gst_amladec_set_caps);
    basetransform_class->start = GST_DEBUG_FUNCPTR ( gst_amladec_start);
    basetransform_class->stop =  GST_DEBUG_FUNCPTR ( gst_amladec_stop);
    basetransform_class->event = GST_DEBUG_FUNCPTR ( gst_amladec_sink_event);
    basetransform_class->before_transform = GST_DEBUG_FUNCPTR ( gst_amladec_before_transform);
    basetransform_class->passthrough_on_same_caps = TRUE;
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_amladec_init (GstAmlAdec * amladec, GstAmlAdecClass * gclass)
{    
    amladec->silent = FALSE;
    AML_DEBUG("gst_amladec_init\n");

}
static void wait_for_render_end (void)
{
    unsigned rp_move_count = 40,count=0;
    unsigned last_rp = 0;
    struct buf_status abuf;
    int ret=1;	
    do {
	if(count>2000)//avoid infinite loop
	      break;	
        ret = codec_get_abuf_state (apcodec, &abuf);
        if (ret != 0) {
            g_print("codec_get_abuf_state error: %x\n", -ret);
            break;
        }
        if(last_rp != abuf.read_pointer){
            last_rp = abuf.read_pointer;
            rp_move_count = 40;
        }else
            rp_move_count--;        
        usleep(1000*30);
        count++;	
    } while (abuf.data_len > 0x100 && rp_move_count > 0);

}

static gboolean
gst_amladec_sink_event  (GstBaseTransform *trans, GstEvent *event)
{
    GstTagList *tag_list;
    GstAmlAdec *amladec = GST_AMLADEC (trans);
    AML_DEBUG( "aout got event %s\n",gst_event_type_get_name (GST_EVENT_TYPE (event))); 
  
    switch (GST_EVENT_TYPE (event)) {  
    case GST_EVENT_NEWSEGMENT:{
        gboolean update;
        gdouble rate;
        GstFormat format;
        gint64 start, stop, time;
        gst_event_parse_new_segment (event, &update, &rate, &format,&start, &stop, &time);      
        break;
    }		
    case GST_EVENT_TAG:
        gst_event_parse_tag (event, &tag_list);
        if (gst_tag_list_is_empty (tag_list))
            g_print("null tag list\n");
        break;	
    case GST_EVENT_FLUSH_STOP:{
        if(amladec->codec_init_ok){
            int res = -1;
            res = codec_reset (apcodec);
            if (res < 0) {
                g_print("reset acodec failed, res= %x\n", res);
                return FALSE;
            }       
        }
        break;
    }		
    case GST_EVENT_FLUSH_START:{  

        break;
    }		
    case GST_EVENT_EOS:
  	  g_print ("adec GST_EVENT_EOS\n"); 
        if (amladec->codec_init_ok)
	 {	
          wait_for_render_end();
	    amladec->is_eos=TRUE;
        }
        break;
    default:
        /* just call the default handler */
        break;
    }
    return parent_class->event (trans, event);
}

static void
gst_amladec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmlAdec *amladec = GST_AMLADEC (object);  
    switch (prop_id) {
        case PROP_SILENT:
            amladec->silent = g_value_get_boolean (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gst_amladec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
    GstAmlAdec *amladec = GST_AMLADEC (object);  
    switch (prop_id) {
        case PROP_SILENT:
            g_value_set_boolean (value,amladec->silent);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static gboolean
gst_amladec_src_query (GstPad  *pad,GstQuery  *query)
{
    gboolean ret;  
    switch (GST_QUERY_TYPE (query)) {
        case GST_QUERY_POSITION:
        /* we should report the current position */     
            break;
        case GST_QUERY_DURATION:
        /* we should report the duration here */   
            break;    
        default:
             ret = FALSE;
            /* just call the default handler */
            //ret = gst_pad_query_default (pad, parent, query);
            break;
    }
    return ret;
}

static gboolean gst_set_astream_info (GstAmlAdec *amladec, GstCaps * caps)
{
    GstStructure *structure;
    const char *name;
    int ret = CODEC_ERROR_NONE;
    int mpegversion; 
    GValue *extra_data_buf = NULL; 
	
    structure = gst_caps_get_structure (caps, 0);
    name=gst_structure_get_name (structure); 
    AML_DEBUG("here caps name =%s,\n",name); 

    if (strcmp(name, "audio/mpeg") == 0) {
        gst_structure_get_int (structure, "mpegversion", &mpegversion); 
        AML_DEBUG("mpegversion=%d\n",mpegversion);      	   
        if (mpegversion==1/*&&layer==3*/) { //mp3
            apcodec->audio_type = AFORMAT_MPEG;
            AML_DEBUG("mp3 info set ok\n");
        }else if (mpegversion==4||mpegversion==2) {
            apcodec->audio_type = AFORMAT_AAC;
            if (gst_structure_has_field (structure, "codec_data")) {	
                extra_data_buf = (GValue *) gst_structure_get_value (structure, "codec_data");
                if (NULL != extra_data_buf) {
                    guint8 *hdrextdata;
                    gint i;
                    amladec->codec_data = gst_value_get_buffer (extra_data_buf);	 
                    AML_DEBUG("AAC SET CAPS check for codec data \n");    
                    amladec->codec_data_len = GST_BUFFER_SIZE (amladec->codec_data);
                    AML_DEBUG("\n>>aac decoder: AAC Codec specific data length is %d\n",amladec->codec_data_len);
                    AML_DEBUG("aac codec data is \n");
                    hdrextdata = GST_BUFFER_DATA (amladec->codec_data);
                    for(i=0;i<amladec->codec_data_len;i++)
                        AML_DEBUG("%x ",hdrextdata[i]);
                    AML_DEBUG("\n");
                    extract_adts_header_info (amladec);
                    hdrextdata = GST_BUFFER_DATA(amladec->codec_data);
                    for(i=0;i<GST_BUFFER_SIZE(amladec->codec_data);i++)
                        AML_DEBUG("%x ",hdrextdata[i]);
                    AML_DEBUG("\n");					
                }
	      }		   
        }		
    }else if (strcmp(name, "audio/x-ac3") == 0) {   	   
        apcodec->audio_type = AFORMAT_AC3;       	 	
    }else if (strcmp(name, "audio/x-adpcm") == 0) {   	   
        apcodec->audio_type = AFORMAT_ADPCM;       	 	
    }else if (strcmp(name, "audio/x-flac") == 0) {   	   
        apcodec->audio_type = AFORMAT_FLAC;       	 	
    }else if (strcmp(name, "audio/x-wma") == 0) {   	   
        apcodec->audio_type = AFORMAT_WMA;       	 	
    }else if (strcmp(name, "audio/x-vorbis") == 0) {   	   
        apcodec->audio_type = AFORMAT_VORBIS;       	 	
    }else if (strcmp(name, "audio/x-mulaw") == 0) {   	   
        apcodec->audio_type = AFORMAT_MULAW;       	 	
    }else if (strcmp(name, "audio/x-raw-int") == 0) {
        gint endianness,depth;
        gboolean getsigned;
        gst_structure_get_int (structure, "endianness", &endianness);
        gst_structure_get_int (structure, "depth", &depth);
        gst_structure_get_int (structure, "rate", &amladec->sample_rate);
        gst_structure_get_int (structure, "channels", &amladec->channels);
        gst_structure_get_boolean (structure, "signed", &getsigned);
        g_print("depth=%d,endianness=%d\n",depth,endianness);
	  if (endianness==1234&&depth==16&&getsigned==true){	
            apcodec->audio_type = AFORMAT_PCM_S16LE;
            amladec->codec_id = CODEC_ID_PCM_S16LE;
        }			
    }else {
        g_print("unsupport audio format name=%s\n",name);
        return FALSE;
   }
	
    if (apcodec&&apcodec->stream_type == STREAM_TYPE_ES_AUDIO){
        if (IS_AUIDO_NEED_EXT_INFO (apcodec->audio_type))
            audioinfo_need_set (apcodec,amladec); 
        ret = codec_init (apcodec);
        if (ret != CODEC_ERROR_NONE){
            g_print("codec init failed, ret=-0x%x", -ret);
            return -1;
        }
        amladec->codec_init_ok = 1;
        g_print("audio codec_init ok,pcodec=%x\n",apcodec);
	set_tsync_enable(1);		
    }    
}

/* this function handles the link with other elements */
static gboolean gst_amladec_set_caps (GstBaseTransform * base, GstCaps * incaps, GstCaps * outcaps)
{
    GstAmlAdec *amladec;  
    
    amladec = GST_AMLADEC (base);
    if(incaps)	
        gst_set_astream_info (amladec, incaps);	
 	
    return TRUE;
}
/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_amladec_render (GstAmlAdec *amladec, GstBuffer * buf)
{ 
    guint8 *data;
    guint size;
    gint written;
    GstClockTime timestamp,pts;

    struct buf_status abuf;
    int ret=1;
	
    if (apcodec&&amladec->codec_init_ok)
    {
        ret = codec_get_abuf_state (apcodec, &abuf);
        if (ret == 0){
            if (abuf.data_len*10 > abuf.size*8){  
                usleep(1000*40);
                //return GST_FLOW_OK;
            }
        }
        timestamp = GST_BUFFER_TIMESTAMP (buf);    
        pts=timestamp*9LL/100000LL+1L;
        if (!amladec->is_headerfeed&&amladec->codec_data_len){
            audiopre_header_feeding (apcodec,amladec,&buf);
        }		
		
        data = GST_BUFFER_DATA (buf);
        size = GST_BUFFER_SIZE (buf);	
        if (timestamp!= GST_CLOCK_TIME_NONE){
            GST_DEBUG_OBJECT (amladec,"pts=%x\n",(unsigned long)pts);
            GST_DEBUG_OBJECT (amladec, "PTS to (%" G_GUINT64_FORMAT ") time: %"
            GST_TIME_FORMAT , pts, GST_TIME_ARGS (timestamp)); 
			
            if (codec_checkin_pts(apcodec,(unsigned long)pts)!=0)
                g_print("pts checkin flied maybe lose sync\n");        	
        }
    	
        again:
    
        GST_DEBUG_OBJECT (amladec, "writing %d bytes to stream buffer r\n", size);
        written=codec_write (apcodec, data, size);
    
        /* check for errors */
        if (G_UNLIKELY (written < 0)) {
          /* try to write again on non-fatal errors */
            if (errno == EAGAIN || errno == EINTR)
                goto again;
            /* else go to our error handler */
            goto write_error;
        }
        /* all is fine when we get here */
        size -= written;
        data += written;
        GST_DEBUG_OBJECT (amladec, "wrote %d bytes, %d left", written, size);
        /* short write, select and try to write the remainder */
        if (G_UNLIKELY (size > 0))
            goto again;   
      
        return GST_FLOW_OK;
    
        write_error:
        {
            switch (errno) {
                case ENOSPC:
                    GST_ELEMENT_ERROR (amladec, RESOURCE, NO_SPACE_LEFT, (NULL), (NULL));
                    break;
                default:{
                   GST_ELEMENT_ERROR (amladec, RESOURCE, WRITE, (NULL),("Error while writing to file  %s",g_strerror (errno)));
                }
            }
            return GST_FLOW_ERROR;
        }
    } else {
        g_print("we will do nothing in render as audio decoder not ready yet\n");

    }	
    return GST_FLOW_OK;
}
/* chain function
 * this function does the actual processing
 */
static  GstFlowReturn gst_amladec_transform_ip (GstBaseTransform *trans, GstBuffer *buf)
{
    GstAmlAdec *amladec;
    amladec = GST_AMLADEC (trans);

    if (amladec->silent == FALSE){  	
        gst_amladec_render (amladec, buf);	
    }	

    /* just push out the incoming buffer without touching it */
    return GST_FLOW_OK;//gst_pad_push (amladec->srcpad, buf);
}

static void gst_amladec_before_transform (GstBaseTransform *trans, GstBuffer *buffer)
{
    GstAmlAdec *amladec;
    GstCaps * caps = NULL;
    amladec = GST_AMLADEC (trans);
    if (!amladec->codec_init_ok&& buffer){
        caps = GST_BUFFER_CAPS(buffer);
        if(caps)	
            gst_set_astream_info (amladec, caps);	
    }
}



static gboolean
gst_amladec_start(GstBaseTransform *trans)
{    
    g_print("start....\n");
    GstAmlAdec *amladec = GST_AMLADEC (trans);
    apcodec = &a_codec_para;
    memset(apcodec, 0, sizeof(codec_para_t ));
    apcodec->audio_pid = 0;
    apcodec->has_audio = 1;
    apcodec->has_video = 0;
    apcodec->audio_channels =0;
    apcodec->audio_samplerate = 0;
    apcodec->noblock = 0;
    apcodec->audio_info.channels = 0;
    apcodec->audio_info.sample_rate = 0;
    apcodec->audio_info.valid = 0;
    apcodec->stream_type = STREAM_TYPE_ES_AUDIO;
    amladec->codec_init_ok = 0;
    amladec->sample_rate = 0;
    amladec->channels = 0;
    amladec->codec_id = 0;
    amladec->bitrate = 0;
    amladec->block_align = 0;
    amladec->is_headerfeed = FALSE;
    amladec->is_paused = FALSE;
    amladec->is_eos = FALSE;
    amladec->codec_init_ok = 0;
    return TRUE;
}

static gboolean
gst_amladec_stop(GstBaseTransform *trans)
{ 
    gint ret = -1;
    g_print("amldec stop,acodec=%x\n",apcodec);
    GstAmlAdec *amladec = GST_AMLADEC (trans);	
    if (amladec->codec_init_ok){
        if (amladec->is_paused == TRUE) {
            ret=codec_resume (apcodec);
            if (ret != 0) {
                g_print("[%s:%d]resume failed!ret=%d\n", __FUNCTION__, __LINE__, ret);
            }else
                amladec->is_paused = FALSE;
        }	
        codec_close (apcodec);
        g_print("acodec=%x\n",apcodec);	  
    }	
    amladec->codec_init_ok=0;	
    return TRUE;
}

static GstStateChangeReturn
gst_amladec_change_state (GstElement * element, GstStateChange transition)
{
    GstAmlAdec *amladec= GST_AMLADEC (element);
    GstStateChangeReturn result;
    gint ret=-1;
    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:            
            break;
        case GST_STATE_CHANGE_READY_TO_PAUSED:
            break;
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:	  	
            if (amladec->is_paused == TRUE) {
                ret=codec_resume (apcodec);
                if (ret != 0) {
                    g_print("[%s:%d]resume failed!ret=%d\n", __FUNCTION__, __LINE__, ret);
                }else
                    amladec->is_paused = FALSE;
            }	
            break;
        default:
            break;
    }

    result = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  
    switch (transition) {
	  case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
            if (!amladec->is_eos)
            {
                ret = codec_pause(apcodec);
                if (ret != 0) {
                    g_print("[%s:%d]pause failed!ret=%d\n", __FUNCTION__, __LINE__, ret);
                }else
                    amladec->is_paused = TRUE;
            }	
	      break;
        case GST_STATE_CHANGE_PAUSED_TO_READY:			
            break;
        case GST_STATE_CHANGE_READY_TO_NULL: 
            break;
        default:
            break;
    }
    return result;
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
amladec_init (GstAmlAdec * amladec)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template amladec' with your description
   */
    GST_DEBUG_CATEGORY_INIT (gst_amladec_debug, "amladec",
        0, "Template amladec");
  
    return gst_element_register (amladec, "amladec", GST_RANK_NONE,
        GST_TYPE_AMLADEC);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstamladec"
#endif

/* gstreamer looks for this structure to register amladecs
 *
 * exchange the string 'Template amladec' with your amladec description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "amladec",
    "Template amladec",
    amladec_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
