/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>

#include <inttypes.h>

#include "gstamlvdec.h"
#include  "gstamlvideoheader.h"
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

//#define AML_DEBUG g_print
#define  AML_DEBUG(...)   GST_INFO_OBJECT(amlvdec,__VA_ARGS__) 

GST_DEBUG_CATEGORY_STATIC (amlvdec_debug);
#define GST_CAT_DEFAULT (amlvdec_debug)

static GstStaticPadTemplate sink_template_factory =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264; video/mpeg; video/x-msmpeg; video/x-h263; video/x-jpeg")
    );

static GstStaticPadTemplate src_template_factory =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw-yuv")
    );

static void gst_amlvdec_base_init (gpointer g_class);
static void gst_amlvdec_class_init (GstAmlVdecClass * klass);
static void gst_amlvdec_init (GstAmlVdec * amlvdec);
static gboolean gst_amlvdec_src_event (GstPad * pad, GstEvent * event);
static gboolean gst_amlvdec_sink_event (GstPad * pad, GstEvent * event);
static gboolean gst_amlvdec_setcaps (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_amlvdec_chain (GstPad * pad, GstBuffer * buf);
static GstStateChangeReturn gst_amlvdec_change_state (GstElement * element,GstStateChange transition);
static GstElementClass *parent_class = NULL;

GType
gst_amlvdec_get_type (void)
{
    static GType amlvdec_type = 0;
    
    if (!amlvdec_type) {
        static const GTypeInfo amlvdec_info = {
            sizeof (GstAmlVdecClass),
            gst_amlvdec_base_init,
            NULL,
            (GClassInitFunc) gst_amlvdec_class_init,
            NULL,
            NULL,
            sizeof (GstAmlVdec),
            0,
            (GInstanceInitFunc) gst_amlvdec_init,
        };    
        amlvdec_type = g_type_register_static (GST_TYPE_ELEMENT, "GstAmlVdec", &amlvdec_info,0);
    }
    
    GST_DEBUG_CATEGORY_INIT (amlvdec_debug, "amlvdec", 0, "AMLVDEC decoder element");    
    return amlvdec_type;
}

static void
gst_amlvdec_base_init (gpointer g_class)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);
  
    gst_element_class_add_static_pad_template (element_class,
        &src_template_factory);
    gst_element_class_add_static_pad_template (element_class,
        &sink_template_factory);
  #ifdef enable_user_data
    gst_element_class_add_static_pad_template (element_class,
        &user_data_template_factory);
  #endif
    gst_element_class_set_details_simple (element_class,
        "aml vdec video decoder", "Codec/Decoder/Video",
        "Uses amlvdec to send video es to hw decode ",
        "aml <aml@aml.org>");
}

static void
gst_amlvdec_class_init (GstAmlVdecClass * klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;
  
    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;  
    parent_class = g_type_class_peek_parent (klass);  
    gstelement_class->change_state = gst_amlvdec_change_state;

}

static void
gst_amlvdec_init (GstAmlVdec * amlvdec)
{
    /* create the sink and src pads */
    amlvdec->sinkpad = gst_pad_new_from_static_template (&sink_template_factory, "sink");
    gst_pad_set_chain_function (amlvdec->sinkpad, GST_DEBUG_FUNCPTR (gst_amlvdec_chain));
    gst_pad_set_event_function (amlvdec->sinkpad, GST_DEBUG_FUNCPTR (gst_amlvdec_sink_event));
    gst_pad_set_setcaps_function (amlvdec->sinkpad, GST_DEBUG_FUNCPTR (gst_amlvdec_setcaps));
    gst_element_add_pad (GST_ELEMENT (amlvdec), amlvdec->sinkpad);
  
    amlvdec->srcpad = gst_pad_new_from_static_template (&src_template_factory, "src");
    gst_pad_set_event_function (amlvdec->srcpad, GST_DEBUG_FUNCPTR (gst_amlvdec_src_event));
    gst_pad_use_fixed_caps (amlvdec->srcpad);
    gst_element_add_pad (GST_ELEMENT (amlvdec), amlvdec->srcpad);

  /* initialize the amlvdec acceleration */
}

static gboolean gst_set_vstream_info (GstAmlVdec  *amlvdec,GstCaps * caps )
{    
    GstStructure  *structure;
    const char  *name;
    gint32 ret = CODEC_ERROR_NONE;
    gint32 mpegversion,msmpegversion;
    GValue *codec_data_buf = NULL; 
	
    structure = gst_caps_get_structure (caps, 0);
    name=gst_structure_get_name (structure); 
    AML_DEBUG("here caps name =%s,\n",name); 
    if (strcmp(name, "video/x-h264") == 0) {
	      gint32 frame_width=0,frame_height=0;
	      gint32 value_numerator=0,value_denominator=0;
        gst_structure_get_int(structure, "width",&frame_width);
        gst_structure_get_int(structure, "height",&frame_height);
	      gst_structure_get_fraction(structure, "framerate",&value_numerator,&value_denominator);
        /* Handle the codec_data information */
        codec_data_buf = (GValue *) gst_structure_get_value(structure, "codec_data"); 	
        if (NULL != codec_data_buf) {
            guint8 *hdrextdata;
            gint i;
            amlvdec->codec_data = gst_value_get_buffer(codec_data_buf);
            g_print("H.264 SET CAPS check for codec data \n");    
            amlvdec->codec_data_len = GST_BUFFER_SIZE(amlvdec->codec_data);
            AML_DEBUG("\n>>H264 decoder: AVC Codec specific data length is %d\n",amlvdec->codec_data_len);
            AML_DEBUG("AVC codec data is \n");
            hdrextdata = GST_BUFFER_DATA(amlvdec->codec_data);
            for(i=0;i<amlvdec->codec_data_len;i++)
                AML_DEBUG("%x ",hdrextdata[i]);
            AML_DEBUG("\n");
        }
		
        amlvdec->pcodec->video_type = VFORMAT_H264;
        amlvdec->pcodec->am_sysinfo.format = VIDEO_DEC_FORMAT_H264;
        amlvdec->pcodec->stream_type = STREAM_TYPE_ES_VIDEO;
        amlvdec->pcodec->am_sysinfo.param = (void *)( EXTERNAL_PTS);
        amlvdec->pcodec->am_sysinfo.height = frame_height;
        amlvdec->pcodec->am_sysinfo.width = frame_width;
        if(value_numerator>0)		
            amlvdec->pcodec->am_sysinfo.rate = 96000*value_denominator/value_numerator;
	      AML_DEBUG(" Frame Width =%d,Height=%d,rate=%d\n", amlvdec->pcodec->am_sysinfo.width, frame_height,amlvdec->pcodec->am_sysinfo.rate);
    }else if (strcmp(name, "video/mpeg") == 0) {  
        gst_structure_get_int (structure, "mpegversion", &mpegversion);
        AML_DEBUG("here mpegversion =%d\n",mpegversion);
        if (mpegversion==2||mpegversion==1) {		
            amlvdec->pcodec->video_type = VFORMAT_MPEG12;
            amlvdec->pcodec->am_sysinfo.format = 0;
        }else if (mpegversion==4){
            gint32 frame_width=0,frame_height=0; 
            gint32 value_numerator=0,value_denominator=0;
            GValue *codec_data_buf = NULL; 
            const gchar *  profile, *getlevel;	
            gst_structure_get_int(structure, "width",&frame_width);
            gst_structure_get_int(structure, "height",&frame_height);
            profile=gst_structure_get_string(structure, "profile");
            getlevel=gst_structure_get_string(structure, "level");
            gst_structure_get_fraction(structure, "framerate",&value_numerator,&value_denominator);
            AML_DEBUG(" Frame Width =%d,Height=%d,profile=%s,getlevel=%s,value_numerator=%d,value_denominator=%d\n", frame_width, frame_height,profile,getlevel,value_numerator,value_denominator);	
            codec_data_buf = (GValue *) gst_structure_get_value(structure, "codec_data");   	
            if (NULL != codec_data_buf) {
                guint8 *hdrextdata;
                gint i;
                amlvdec->codec_data = gst_value_get_buffer(codec_data_buf);
                g_print("mp4v check for codec data \n");    
                amlvdec->codec_data_len = GST_BUFFER_SIZE(amlvdec->codec_data);
                AML_DEBUG("\n>>mp4v Codec specific data length is %d\n",amlvdec->codec_data_len);
                AML_DEBUG("mp4v codec data is \n");
                hdrextdata = GST_BUFFER_DATA(amlvdec->codec_data);
                for(i=0;i<amlvdec->codec_data_len;i++)
                    AML_DEBUG("%x ",hdrextdata[i]);
                AML_DEBUG("\n");    
            }
            amlvdec->pcodec->video_type = VFORMAT_MPEG4;
            amlvdec->pcodec->am_sysinfo.format = VIDEO_DEC_FORMAT_MPEG4_5;
            amlvdec->pcodec->am_sysinfo.height = frame_height;
            amlvdec->pcodec->am_sysinfo.width = frame_width;
            if(value_numerator>0)		
               amlvdec->pcodec->am_sysinfo.rate = 96000*value_denominator/value_numerator;		 
            }		
    }else if (strcmp(name, "video/x-msmpeg") == 0) {
        gst_structure_get_int (structure, "msmpegversion", &msmpegversion);
        if (msmpegversion==43){
            amlvdec->pcodec->video_type = VFORMAT_MPEG4;
            amlvdec->pcodec->am_sysinfo.format = VIDEO_DEC_FORMAT_MPEG4_3;
        }
    }else if (strcmp(name, "video/x-h263") == 0) {        
        amlvdec->pcodec->video_type = VFORMAT_MPEG4;
        amlvdec->pcodec->am_sysinfo.format = VIDEO_DEC_FORMAT_H263;        
    }else if (strcmp(name, "video/x-jpeg") == 0) {        
        amlvdec->pcodec->video_type = VFORMAT_MJPEG;
        amlvdec->pcodec->am_sysinfo.format = VIDEO_DEC_FORMAT_MJPEG;        
    }else {
        g_print("unsupport video format, %s",name);
        return FALSE;	
    }
    if (amlvdec->pcodec&&amlvdec->pcodec->stream_type == STREAM_TYPE_ES_VIDEO){ 
        AML_DEBUG("pcodec->videotype=%d\n",amlvdec->pcodec->video_type);	
        ret = codec_init(amlvdec->pcodec);
         if (ret != CODEC_ERROR_NONE){
             AML_DEBUG("codec init failed, ret=-0x%x", -ret);
             return FALSE;
        }
        set_fb0_blank(1);
        set_fb1_blank(1);
        set_tsync_enable(1);
        amlvdec->codec_init_ok=1;
        AML_DEBUG("video codec_init ok\n");
    } 	
    return TRUE;	
}

static GstFlowReturn
gst_amlvdec_decode (GstAmlVdec *amlvdec, GstBuffer * buf)
{
    guint8 *data,ret;
    guint size;
    gint written;
    GstClockTime timestamp,pts;
    struct buf_status vbuf;
    GstCaps * caps = NULL;

    if (!amlvdec->codec_init_ok){
        caps = GST_BUFFER_CAPS (buf);
        if (caps)		
            gst_set_vstream_info (amlvdec, caps );
    }

    if(amlvdec->codec_init_ok)
    {   
        ret = codec_get_vbuf_state(amlvdec->pcodec, &vbuf);
        if(ret == 0){
            if(vbuf.data_len*10 > vbuf.size*8){  
                usleep(1000*40);
                //return GST_FLOW_OK;
            }
        }
		
        timestamp = GST_BUFFER_TIMESTAMP (buf);
        pts=timestamp*9LL/100000LL+1L;        
        if(!amlvdec->is_headerfeed&&amlvdec->codec_data_len){ 	
            videopre_header_feeding(amlvdec->pcodec,amlvdec,&buf);
            amlvdec->is_headerfeed=TRUE;   
        }else if(VFORMAT_H264 == amlvdec->pcodec->video_type) 
            h264_update_frame_header(&buf);
		
        data = GST_BUFFER_DATA (buf);
        size = GST_BUFFER_SIZE (buf);
        if (timestamp!= GST_CLOCK_TIME_NONE){
            GST_DEBUG_OBJECT (amlvdec,"pts=%x\n",(unsigned long)pts);
            GST_DEBUG_OBJECT (amlvdec, "PTS to (%" G_GUINT64_FORMAT ") time: %"
            GST_TIME_FORMAT , pts, GST_TIME_ARGS (timestamp)); 
            
            if(codec_checkin_pts(amlvdec->pcodec,(unsigned long)pts)!=0)
                AML_DEBUG("pts checkin flied maybe lose sync\n");  
        }
    	
        again:    
        GST_DEBUG_OBJECT (amlvdec, "writing %d bytes to stream buffer r\n", size);
        written=codec_write(amlvdec->pcodec, data, size);
    
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
        GST_DEBUG_OBJECT (amlvdec, "wrote %d bytes, %d left", written, size);
        /* short write, select and try to write the remainder */
        if (G_UNLIKELY (size > 0))
            goto again;  
      
        return GST_FLOW_OK;    
        write_error:
        {
            switch (errno) {
                case ENOSPC:
                    GST_ELEMENT_ERROR (amlvdec, RESOURCE, NO_SPACE_LEFT, (NULL), (NULL));
                    break;
                default:{
                    GST_ELEMENT_ERROR (amlvdec, RESOURCE, WRITE, (NULL),("Error while writing to file  %s",g_strerror (errno)));
                }
            }
            return GST_FLOW_ERROR;
        }		
    }else{
        AML_DEBUG("decoder not init ok yet, we will do nothing in render\n");
    }
    return GST_FLOW_OK;
}

static GstFlowReturn
gst_amlvdec_chain (GstPad * pad, GstBuffer * buf)
{
    GstAmlVdec *amlvdec;  
    amlvdec = GST_AMLVDEC(GST_PAD_PARENT (pad));
  
    if (amlvdec->silent == FALSE){  	
        gst_amlvdec_decode (amlvdec, buf);	
    }
    return gst_pad_push (amlvdec->srcpad, buf);
  
    /* just push out the incoming buffer without touching it */
  //  return GST_FLOW_OK;
}
static void wait_for_render_end(GstAmlVdec *amlvdec)
{
    unsigned rp_move_count = 40,count=0;
    struct buf_status vbuf;
    unsigned last_rp = 0;
    int ret=1;	
    do {
	  if(count>2000)//avoid infinite loop
	      break;	
        ret = codec_get_vbuf_state(amlvdec->pcodec, &vbuf);
        if (ret != 0) {
            g_print("codec_get_vbuf_state error: %x\n", -ret);
            break;
        }
        if(last_rp != vbuf.read_pointer){
            last_rp = vbuf.read_pointer;
            rp_move_count = 40;
        }else
            rp_move_count--;        
            usleep(1000*30);
            count++;	
    } while (vbuf.data_len > 0x100 && rp_move_count > 0);
}

static gboolean amlvdec_forward_process(GstAmlVdec *amlvdec, 
    gboolean update, gdouble rate, GstFormat format, gint64 start,
    gint64 stop, gint64 position)
{
    AmlState eCurrentState = AmlStateNormal;
    if(((rate - 1.0) < 0.000001) && ((rate - 1.0) > -0.000001)){
        set_tsync_enable(1);
        codec_set_video_playrate(amlvdec->pcodec, (int)(rate*(1<<16)));
        eCurrentState = AmlStateNormal;
        amlvdec->trickRate = rate;
    }
    else if(rate > 0){
        set_tsync_enable(0);
        codec_set_video_playrate(amlvdec->pcodec, (int)(rate*(1<<16)));
        if(rate > 1.0){
            eCurrentState = AmlStateFastForward;
        }
        else{
            eCurrentState = AmlStateSlowForward;
        }
        amlvdec->trickRate = rate;
    }

    if(eCurrentState != amlvdec->eState){
        amlvdec->eState = eCurrentState;
    }
    
    return TRUE;
}

static gboolean
gst_amlvdec_sink_event (GstPad * pad, GstEvent * event)
{
    GstAmlVdec *amlvdec;
    gboolean ret = TRUE;
    amlvdec = GST_AMLVDEC(gst_pad_get_parent (pad));
    GST_DEBUG_OBJECT (amlvdec, "Got %s event on sink pad", GST_EVENT_TYPE_NAME (event));
    switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_NEWSEGMENT:
        {
            gboolean update;
            GstFormat format;
            gdouble rate, arate;
            gint64 start, stop, time;

            gst_event_parse_new_segment_full (event, &update, &rate, &arate, &format, &start, &stop, &time);

            if (format != GST_FORMAT_TIME)
                goto newseg_wrong_format;
            amlvdec_forward_process(amlvdec, update, rate, format, start, stop, time);
            gst_segment_set_newsegment_full (&amlvdec->segment, update, rate, arate, format, start, stop, time);

            GST_DEBUG_OBJECT (amlvdec,"Pushing newseg rate %g, applied rate %g, format %d, start %"
                G_GINT64_FORMAT ", stop %" G_GINT64_FORMAT ", pos %" G_GINT64_FORMAT,
                rate, arate, format, start, stop, time);

            ret = gst_pad_push_event (amlvdec->srcpad, event);
            break;
        }
		
        case GST_EVENT_FLUSH_START:
            
            if(amlvdec->codec_init_ok){
                set_black_policy(0);
            }
            ret = gst_pad_push_event (amlvdec->srcpad, event);
            break;
	  
        case GST_EVENT_FLUSH_STOP:
        {
            if(amlvdec->codec_init_ok){
                gint res = -1;
                res = codec_reset(amlvdec->pcodec);
                if (res < 0) {
                    g_print("reset vcodec failed, res= %x\n", res);
                    return FALSE;
                }            
                amlvdec->is_headerfeed = FALSE; 
            }
            ret = gst_pad_push_event (amlvdec->srcpad, event);
            break;
        } 
		
        case GST_EVENT_EOS:
            AML_DEBUG("ge GST_EVENT_EOS,check for video end\n");
            if(amlvdec->codec_init_ok)	
            {
                wait_for_render_end(amlvdec);
                amlvdec->is_eos = TRUE;
            }	
            ret = gst_pad_push_event (amlvdec->srcpad, event);
            break;
		 
        default:
            ret = gst_pad_push_event (amlvdec->srcpad, event);
            break;
    }

done:
    gst_object_unref (amlvdec);

    return ret;

  /* ERRORS */
newseg_wrong_format:
  {
    GST_DEBUG_OBJECT (amlvdec, "received non TIME newsegment");
    gst_event_unref (event);
    goto done;
  }
}


static gboolean
gst_amlvdec_setcaps (GstPad * pad, GstCaps * caps)
{
    GstAmlVdec *amlvdec;
    GstPad *otherpad;
    amlvdec = GST_AMLVDEC (gst_pad_get_parent (pad));
    otherpad = (pad == amlvdec->srcpad) ? amlvdec->sinkpad : amlvdec->srcpad;
    if(caps)	
        gst_set_vstream_info (amlvdec, caps );  
    gst_object_unref (amlvdec);
  
    return TRUE;
}

static gboolean
gst_amlvdec_src_event (GstPad * pad, GstEvent * event)
{
    gboolean res;
    GstAmlVdec *amlvdec;  
    amlvdec =  GST_AMLVDEC (GST_PAD_PARENT (pad));
  
    switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_SEEK:{
            gst_event_ref (event);
            res = gst_pad_push_event (amlvdec->sinkpad, event) ;
            gst_event_unref (event);
            break;
        }		

        default:
            res = gst_pad_push_event (amlvdec->sinkpad, event);
            break;
    }	
    return res; 
}

static gboolean
gst_amlvdec_start (GstAmlVdec *amlvdec)
{ 
    AML_DEBUG("amlvdec start....\n");
    amlvdec->codec_init_ok=0;
    //amlvdec->pcodec = &v_codec_para;
    amlvdec->pcodec = g_malloc(sizeof(codec_para_t));
    memset(amlvdec->pcodec, 0, sizeof(codec_para_t ));
    amlvdec->pcodec->has_video = 1;
    amlvdec->pcodec->am_sysinfo.rate = 0;
    amlvdec->pcodec->am_sysinfo.height = 0;
    amlvdec->pcodec->am_sysinfo.width = 0;  
    amlvdec->pcodec->has_audio = 0;
    amlvdec->pcodec->noblock = 0;
    amlvdec->pcodec->stream_type = STREAM_TYPE_ES_VIDEO;
    amlvdec->is_headerfeed = FALSE;
    amlvdec->codec_data_len = 0;
    amlvdec->codec_data = NULL;
    amlvdec->is_paused = FALSE;
    amlvdec->is_eos = FALSE;
    amlvdec->codec_init_ok = 0;
    amlvdec->prival = 0;
    amlvdec->trickRate = 1.0;
    return TRUE;
}

static gboolean
gst_amlvdec_stop (GstAmlVdec *amlvdec)
{
    gint ret = -1;
    if(amlvdec->codec_init_ok){
        if(amlvdec->is_paused == TRUE) {
            ret=codec_resume(amlvdec->pcodec);
            if (ret != 0) {
                g_print("[%s:%d]resume failed!ret=%d\n", __FUNCTION__, __LINE__, ret);
            }else
                amlvdec->is_paused = FALSE;
        }	
        set_black_policy(1);
        codec_close(amlvdec->pcodec);
    }
    amlvdec->codec_init_ok=0;
    amlvdec->is_headerfeed=FALSE;
    return TRUE;
}

static GstStateChangeReturn
gst_amlvdec_change_state (GstElement * element, GstStateChange transition)
{
    GstStateChangeReturn result;
    GstAmlVdec *amlvdec =  GST_AMLVDEC(element);
    GstAmlVdecClass *amlclass = GST_AMLVDEC_GET_CLASS (amlvdec); 
    GstElementClass *parent_class = g_type_class_peek_parent (amlclass);
    gint ret= -1;
    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:
            gst_amlvdec_start(amlvdec);
            break;
  		
        case GST_STATE_CHANGE_READY_TO_PAUSED:
    
            break;
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
            if(amlvdec->is_paused == TRUE &&  amlvdec->codec_init_ok) {
                ret=codec_resume(amlvdec->pcodec);
                if (ret != 0) {
                    g_print("[%s:%d]resume failed!ret=%d\n", __FUNCTION__, __LINE__, ret);
                }else
                    amlvdec->is_paused = FALSE;
            }

        default:
          break;
    }

    result =  parent_class->change_state (element, transition);

    switch (transition) {
        case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
            if(!amlvdec->is_eos &&  amlvdec->codec_init_ok){ 
                ret=codec_pause(amlvdec->pcodec);
                if (ret != 0) {
                    g_print("[%s:%d]pause failed!ret=%d\n", __FUNCTION__, __LINE__, ret);
                }else
                    amlvdec->is_paused = TRUE;
            }
            break;
			
        case GST_STATE_CHANGE_PAUSED_TO_READY:
    
          break;
		  
        case GST_STATE_CHANGE_READY_TO_NULL:
            gst_amlvdec_stop(amlvdec);
            break;
			
        default:
            break;
    }

    return result;
  
}

static gboolean
plugin_init (GstPlugin * plugin)
{
    if (!gst_element_register (plugin, "amlvdec", GST_RANK_PRIMARY,
            GST_TYPE_AMLVDEC))
      return FALSE;
  
    return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "amlvdec",
    "aml fake video decoder",
    plugin_init,
    VERSION, 
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/");
