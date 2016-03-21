/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2015 U-AMLOGICTao.Guo <<user@hostname.org>>
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
 * SECTION:element-amlvdec
 *
 * FIXME:Describe amlvdec here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amlvdec ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <gst/gst.h>

#include "gstamlvdec.h"

GST_DEBUG_CATEGORY_STATIC (gst_aml_vdec_debug);
#define GST_CAT_DEFAULT gst_aml_vdec_debug
#define VERSION	"1.1"

#define COMMON_VIDEO_CAPS \
  "width = (int) [ 16, 4096 ], " \
  "height = (int) [ 16, 4096 ] "

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
	GST_STATIC_CAPS (
			"video/mpeg, "
	        "mpegversion = (int) { 1, 2, 4 }, "
	        "systemstream = (boolean) false, "
	        COMMON_VIDEO_CAPS "; "
	          "video/x-h264, "
	        "stream-format= byte-stream, "
	        "alignment=nal " "; "
	        "video/x-h264, "
	        "stream-format={ avc, byte-stream }, "
	        "alignment={ au, nal }, "
	        COMMON_VIDEO_CAPS "; "
	        "video/x-h265, "
	        COMMON_VIDEO_CAPS "; "
	        "video/x-divx, "
	        COMMON_VIDEO_CAPS "; "
//	        "video/x-huffyuv, "
//	        COMMON_VIDEO_CAPS "; "
//	        "video/x-dv, "
//	        COMMON_VIDEO_CAPS "; "
	        "video/x-h263, "
	        COMMON_VIDEO_CAPS "; "
	        "video/x-msmpeg, "
	        COMMON_VIDEO_CAPS "; "
	        "image/jpeg, "
	        COMMON_VIDEO_CAPS "; "
//	        "video/x-theora; "
//	        "video/x-dirac, "
//	        COMMON_VIDEO_CAPS "; "
//	        "video/x-pn-realvideo, "
//	        "rmversion = (int) [1, 4], "
//	        COMMON_VIDEO_CAPS "; "
//	        "video/x-vp8, "
//	        COMMON_VIDEO_CAPS "; "
//	        "video/x-vp9, "
//	        COMMON_VIDEO_CAPS "; "
//	        "video/x-raw, "
//	        "format = (string) { YUY2, I420, YV12, UYVY, AYUV, GRAY8, BGR, RGB }, "
//	        COMMON_VIDEO_CAPS "; "
               "video/x-pn-realvideo; " 
	        "video/x-wmv, "
			"wmvversion = (int) [ 1, 3 ], "
			COMMON_VIDEO_CAPS)
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
	GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("xRGB"))
    );


static void 					gst_aml_vdec_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void 					gst_aml_vdec_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean 				gst_aml_vdec_open(GstVideoDecoder * dec);
static gboolean 				gst_aml_vdec_close(GstVideoDecoder * dec);
static gboolean 				gst_aml_vdec_start(GstVideoDecoder * dec);
static gboolean 				gst_aml_vdec_stop(GstVideoDecoder * dec);
static gboolean 				gst_aml_vdec_set_format(GstVideoDecoder *dec, GstVideoCodecState *state);
static GstFlowReturn 			gst_aml_vdec_handle_frame(GstVideoDecoder *dec, GstVideoCodecFrame *frame);
static void 					gst_aml_vdec_flush(GstVideoDecoder * dec);

static gboolean 				gst_set_vstream_info(GstAmlVdec *amlvdec, GstCaps * caps);
static GstFlowReturn 			gst_aml_vdec_decode (GstAmlVdec *amlvdec, GstBuffer * buf);

#define gst_aml_vdec_parent_class parent_class
G_DEFINE_TYPE (GstAmlVdec, gst_aml_vdec, GST_TYPE_VIDEO_DECODER);

/* GObject vmethod implementations */

/* initialize the amlvdec's class */
static void
gst_aml_vdec_class_init (GstAmlVdecClass * klass)
{
	GObjectClass *gobject_class = (GObjectClass *) klass;
	GstElementClass *element_class = (GstElementClass *) klass;
	GstVideoDecoderClass *base_class = (GstVideoDecoderClass *) klass;

	gobject_class->set_property = gst_aml_vdec_set_property;
	gobject_class->get_property = gst_aml_vdec_get_property;

	gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&sink_factory));
	gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&src_factory));

	gst_element_class_set_details_simple(element_class,
			"Amlogic Video Decoder",
			"Codec/Decoder/Video",
			"Amlogic Video Decoder Plugin",
			"mm@amlogic.com");

	base_class->open = GST_DEBUG_FUNCPTR(gst_aml_vdec_open);
	base_class->close = GST_DEBUG_FUNCPTR(gst_aml_vdec_close);
	base_class->start = GST_DEBUG_FUNCPTR(gst_aml_vdec_start);
	base_class->stop = GST_DEBUG_FUNCPTR(gst_aml_vdec_stop);
	base_class->set_format = GST_DEBUG_FUNCPTR(gst_aml_vdec_set_format);
	base_class->handle_frame = GST_DEBUG_FUNCPTR(gst_aml_vdec_handle_frame);
	base_class->flush = GST_DEBUG_FUNCPTR(gst_aml_vdec_flush);


}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_aml_vdec_init (GstAmlVdec * amlvdec)
{
	GstVideoDecoder *dec;
	dec = GST_VIDEO_DECODER (amlvdec);
}

static void
gst_aml_vdec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
	GstAmlVdec *amlvdec = GST_AMLVDEC(object);

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
gst_aml_vdec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
	GstAmlVdec *amlvdec = GST_AMLVDEC(object);

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean
gst_aml_vdec_open(GstVideoDecoder * dec)
{
	GstAmlVdec *amlvdec = GST_AMLVDEC(dec);

	amlvdec->pcodec = g_malloc(sizeof(codec_para_t));
	memset(amlvdec->pcodec, 0, sizeof(codec_para_t));
#if DEBUG_DUMP
	amlvdec->dump_fd = open("/data/local/tmp/gst_aml_vdec.dump", O_CREAT | O_TRUNC | O_WRONLY, 0777);
	if (amlvdec->dump_fd < 0) {
		GST_ERROR("fd_dump open failed");
	} else {
		codec_set_dump_fd(amlvdec->pcodec, amlvdec->dump_fd);
	}
#endif
	return TRUE;
}
static gboolean
gst_aml_vdec_close(GstVideoDecoder * dec)
{
	GstAmlVdec *amlvdec = GST_AMLVDEC(dec);
#if DEBUG_DUMP
	if (amlvdec->dump_fd > 0) {
		codec_set_dump_fd(NULL, 0);
		close(amlvdec->dump_fd);
	}
#endif
	if (amlvdec->pcodec) {
		g_free(amlvdec->pcodec);
		amlvdec->pcodec = NULL;
	}
	return TRUE;
}
static gboolean
gst_aml_vdec_start(GstVideoDecoder * dec)
{
	GstAmlVdec *amlvdec = GST_AMLVDEC(dec);

	amlvdec->pcodec->has_video = 1;
	amlvdec->pcodec->am_sysinfo.rate = 0;
	amlvdec->pcodec->am_sysinfo.height = 0;
	amlvdec->pcodec->am_sysinfo.width = 0;
	amlvdec->pcodec->has_audio = 0;
	amlvdec->pcodec->noblock = 0;
	amlvdec->pcodec->stream_type = STREAM_TYPE_ES_VIDEO;
	amlvdec->is_headerfeed = FALSE;
	amlvdec->is_paused = FALSE;
	amlvdec->is_eos = FALSE;
	amlvdec->codec_init_ok = 0;
	amlvdec->trickRate = 1.0;
	return TRUE;
}

static gboolean
gst_aml_vdec_stop(GstVideoDecoder * dec)
{
	gint ret = -1;
	GstAmlVdec *amlvdec = GST_AMLVDEC(dec);

	if (amlvdec->codec_init_ok) {
		amlvdec->codec_init_ok = 0;
		if (amlvdec->is_paused == TRUE) {
			ret = codec_resume(amlvdec->pcodec);
			if (ret != 0) {
				GST_ERROR("[%s:%d]resume failed!ret=%d", __FUNCTION__, __LINE__, ret);
			} else
				amlvdec->is_paused = FALSE;
		}
		set_black_policy(1);
		codec_close(amlvdec->pcodec);

		amlvdec->is_headerfeed = FALSE;
		if (amlvdec->input_state) {
			gst_video_codec_state_unref(amlvdec->input_state);
			amlvdec->input_state = NULL;
		}
		
		set_fb0_blank(0);
		set_fb1_blank(0);
//T?		set_display_axis(1);
	}
	if (amlvdec->info) {
			amlvdec->info->finalize(amlvdec->info);
			amlvdec->info = NULL;
		}
	
	return TRUE;
}
static gboolean gst_aml_vdec_set_format(GstVideoDecoder *dec, GstVideoCodecState *state)
{
	gboolean ret = FALSE;
	GstStructure *structure;
	const char *name;
	GstVideoInfo *info;
	gint par_num, par_den;
	GstVideoFormat fmt;
	GstAmlVdec *amlvdec = GST_AMLVDEC(dec);

	g_return_val_if_fail(state != NULL, FALSE);
	if (amlvdec->input_state)
		gst_video_codec_state_unref(amlvdec->input_state);
	amlvdec->input_state = gst_video_codec_state_ref(state);

	structure = gst_caps_get_structure(state->caps, 0);
	name = gst_structure_get_name(structure);
	GST_INFO_OBJECT(amlvdec, "%s = %s", __FUNCTION__, name);
	if (name) {
		ret = gst_set_vstream_info(amlvdec, state->caps);
		if (!amlvdec->output_state) {
			info = &amlvdec->input_state->info;
			fmt = GST_VIDEO_FORMAT_xRGB;
			GST_VIDEO_INFO_WIDTH (info) = amlvdec->pcodec->am_sysinfo.width;
			GST_VIDEO_INFO_HEIGHT (info) = amlvdec->pcodec->am_sysinfo.height;
			par_num = GST_VIDEO_INFO_PAR_N(info);
			par_den = GST_VIDEO_INFO_PAR_D(info);
			amlvdec->output_state = gst_video_decoder_set_output_state(GST_VIDEO_DECODER(amlvdec),
					fmt, info->width,
					info->height,
					amlvdec->input_state);
			gst_video_decoder_negotiate (GST_VIDEO_DECODER (amlvdec));
		}

	}
	return ret;
}

static GstFlowReturn
gst_aml_vdec_handle_frame(GstVideoDecoder *dec, GstVideoCodecFrame *frame)
{
	GstAmlVdec *amlvdec = GST_AMLVDEC(dec);
	GstBuffer *outbuffer;
	GstFlowReturn ret;
	GstVideoCodecState *state;

	GST_DEBUG_OBJECT(dec, "%s %d", __FUNCTION__, __LINE__);

	if (G_UNLIKELY(!frame))
		return GST_FLOW_OK;

	ret = gst_video_decoder_allocate_output_frame(dec, frame);
	if (G_UNLIKELY(ret != GST_FLOW_OK)) {
		GST_ERROR_OBJECT(dec, "failed to allocate output frame");
		ret = GST_FLOW_ERROR;
		gst_video_codec_frame_unref(frame);
		goto done;
	}

	gst_aml_vdec_decode(amlvdec, frame->input_buffer);
	gst_video_decoder_finish_frame(dec, frame);
//	gst_video_codec_frame_unref(frame);
//	gst_video_decoder_release_frame(dec, frame);
	ret = GST_FLOW_OK;
done:
	return ret;
}


static void
gst_aml_vdec_flush(GstVideoDecoder * dec)
{
	GstAmlVdec *amlvdec = GST_AMLVDEC(dec);
	codec_reset(amlvdec->pcodec);
}

static gboolean
gst_set_vstream_info(GstAmlVdec *amlvdec, GstCaps * caps)
{
	GstStructure *structure;
	const char *name;
	gint32 ret = CODEC_ERROR_NONE;
	AmlStreamInfo *videoinfo = NULL;
	int coordinate[4] = {0, 0, 0, 0};
	structure = gst_caps_get_structure(caps, 0);
	name = gst_structure_get_name(structure);

	videoinfo = amlVstreamInfoInterface(name);
	if (NULL == name) {
		return FALSE;
	}
	amlvdec->info = videoinfo;
	videoinfo->init(videoinfo, amlvdec->pcodec, structure);
	if (amlvdec->pcodec && amlvdec->pcodec->stream_type == STREAM_TYPE_ES_VIDEO) {
		if (!amlvdec->codec_init_ok) {
			ret = codec_init(amlvdec->pcodec);
			if (ret != CODEC_ERROR_NONE) {
				GST_ERROR("codec init failed, ret=-0x%x", -ret);
				return FALSE;
			}
			set_fb0_blank(1);
			set_fb1_blank(1);
			set_tsync_enable(1);
			set_display_axis(coordinate);
			//set_video_axis(coordinate);
			amlvdec->codec_init_ok = 1;
			if (amlvdec->trickRate > 0) {
				if (amlvdec->pcodec && amlvdec->pcodec->cntl_handle) {
//T					codec_set_video_playrate(amlvdec->pcodec, (int) (amlvdec->trickRate * (1 << 16)));
				}
			}
			GST_DEBUG_OBJECT(amlvdec, "video codec_init ok");
		}

	}
	return TRUE;
}

static GstFlowReturn
gst_aml_vdec_decode (GstAmlVdec *amlvdec, GstBuffer * buf)
{
	GstFlowReturn ret = GST_FLOW_OK;
	guint8 *data;
	guint size;
	gint written;
	GstClockTime timestamp, pts;

	struct buf_status vbuf;
	GstMapInfo map;

	if (!amlvdec->info) {
		return GST_FLOW_OK;
	}
	if (amlvdec->pcodec && amlvdec->codec_init_ok) {
		while (codec_get_vbuf_state(amlvdec->pcodec, &vbuf) == 0) {
			if (vbuf.data_len * 10 < vbuf.size * 7) {
				break;
			}
			if (amlvdec->is_paused) {
				break;
			}
			usleep(20000);
		}
		timestamp = GST_BUFFER_TIMESTAMP (buf);
		pts = timestamp * 9LL / 100000LL + 1L;

		if (timestamp != GST_CLOCK_TIME_NONE) {
			GST_INFO_OBJECT(amlvdec, " video pts = %x", (unsigned long) pts);
			if (codec_checkin_pts(amlvdec->pcodec, (unsigned long) pts) != 0)
				GST_ERROR_OBJECT(amlvdec, "pts checkin flied maybe lose sync");

		}

		if (!amlvdec->is_headerfeed) {
			if (amlvdec->info->writeheader) {
				amlvdec->info->writeheader(amlvdec->info, amlvdec->pcodec);
			}
			amlvdec->is_headerfeed = TRUE;
		}
		if (amlvdec->info->add_startcode) {
			amlvdec->info->add_startcode(amlvdec->info, amlvdec->pcodec, buf);
		}
		gst_buffer_map(buf, &map, GST_MAP_READ);
		data = map.data;
		size = map.size;
		while (size > 0) {
			written = codec_write(amlvdec->pcodec, data, size);
			if (written >= 0) {
				size -= written;
				data += written;
			} else if (errno == EAGAIN || errno == EINTR) {
				GST_WARNING_OBJECT(amlvdec, "codec_write busy");
				if (amlvdec->is_paused) {
					break;
				}
				usleep(20000);
			} else {
				GST_ERROR_OBJECT(amlvdec, "codec_write failed");
				ret = GST_FLOW_ERROR;
				break;
			}
		}
		gst_buffer_unmap(buf, &map);
	}
	return ret;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
amlvdec_init (GstPlugin * amlvdec)
{
	/* debug category for fltering log messages
	 *
	 * exchange the string 'Template amlvdec' with your description
	 */
	GST_DEBUG_CATEGORY_INIT(gst_aml_vdec_debug, "amlvdec", 0, "Amlogic Video Decoder");

	return gst_element_register(amlvdec, "amlvdec", GST_RANK_PRIMARY, GST_TYPE_AMLVDEC);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "gst-plugins-amlogic"
#endif

/* gstreamer looks for this structure to register amlvdecs
 *
 * exchange the string 'Template amlvdec' with your amlvdec description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amlvdec,
    "Amlogic Video Decoder",
    amlvdec_init,
    VERSION,
    "LGPL",
    "Amlogic",
    "http://amlogic.com/"
)
