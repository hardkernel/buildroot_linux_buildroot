/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2018 Jemy Zhang <<jun.zhang@amlogic.com>>
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
 * SECTION:element-amlvenc
 *
 * FIXME:Describe amlvenc here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amlvenc ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gst/pbutils/pbutils.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include <gst/video/gstvideosink.h>
#include <gmodule.h>
#include <string.h>

#include "gstamlvenc.h"

GST_DEBUG_CATEGORY_STATIC (gst_amlvenc_debug);
#define GST_CAT_DEFAULT gst_amlvenc_debug

#define AMLH264_ENCODER_LIBRARIES "/usr/lib/libvpcodec.so"
#define AMLH265_ENCODER_LIBRARIES "/usr/lib/libvphevcodec.so"


typedef enum vl_codec_id_e {
  CODEC_ID_NONE,
  CODEC_ID_VP8,
  CODEC_ID_H261,
  CODEC_ID_H263,
  CODEC_ID_H264,
  CODEC_ID_H265,

} vl_codec_id_t;

typedef enum vl_img_format_e
{
  IMG_FMT_NONE,
  IMG_FMT_NV12,
  IMG_FMT_NV21,
  IMG_FMT_YV12,
} vl_img_format_t;

typedef enum vl_frame_type_e
{
  FRAME_TYPE_NONE,
  FRAME_TYPE_AUTO, /* encoder self-adaptation(default) */
  FRAME_TYPE_IDR,
  FRAME_TYPE_I,
  FRAME_TYPE_P,

} vl_frame_type_t;


struct _GstAmlVEncVTable
{
  GModule *module;
  gint codec_id;

  vl_codec_handle_t (*vl_video_encoder_init)(vl_codec_id_t codec_id, int width, int height, int frame_rate, int bit_rate, int gop, vl_img_format_t img_format);
  int (*vl_video_encoder_encode)(vl_codec_handle_t codec_handle, int frame_type, unsigned char *in, int out_buf_size, unsigned char *out, int format);
  int (*vl_video_encoder_destory)(vl_codec_handle_t codec_handle);
};

static GstAmlVEncVTable *vtable_aml = NULL;

#define LOAD_SYMBOL(name) G_STMT_START { \
  if (!g_module_symbol (module, #name, (gpointer *) &vtable->name)) { \
    GST_ERROR ("Failed to load '" #name "' from '%s'", filename); \
    goto error; \
  } \
} G_STMT_END;

static GstAmlVEncVTable *
load_v (const gchar * filename)
{
  GModule *module;
  GstAmlVEncVTable *vtable;

  module = g_module_open (filename, G_MODULE_BIND_LOCAL);
  if (!module) {
    GST_ERROR ("Failed to load '%s'", filename);
    return NULL;
  }

  vtable = g_new0 (GstAmlVEncVTable, 1);
  vtable->module = module;

  if (!g_module_symbol (module, G_STRINGIFY (vl_video_encoder_init),
          (gpointer *) & vtable->vl_video_encoder_init)) {
    GST_ERROR ("Failed to load '" G_STRINGIFY (vl_video_encoder_init)
        "' from '%s'. Incompatible version?", filename);
    goto error;
  }

  LOAD_SYMBOL (vl_video_encoder_encode);
  LOAD_SYMBOL (vl_video_encoder_destory);

  return vtable;

error:
  g_module_close (vtable->module);
  g_free (vtable);
  return NULL;
}

static void
unload_v (GstAmlVEncVTable * vtable)
{
  if (vtable->module) {
    g_module_close (vtable->module);
    g_free (vtable);
  }
}

#undef LOAD_SYMBOL

static gboolean
gst_amlvenc_add_v_chroma_format (GstStructure * s)
{
  GValue fmts = G_VALUE_INIT;
  GValue fmt = G_VALUE_INIT;
  gboolean ret = FALSE;

  g_value_init (&fmts, GST_TYPE_LIST);
  g_value_init (&fmt, G_TYPE_STRING);

  g_value_set_string (&fmt, "NV21");
  gst_value_list_append_value (&fmts, &fmt);
  g_value_set_string (&fmt, "RGB");
  gst_value_list_append_value (&fmts, &fmt);
  g_value_set_string (&fmt, "I420");
  gst_value_list_append_value (&fmts, &fmt);

  if (vtable_aml) {
    if (vtable_aml->codec_id == CODEC_ID_H265) {
      g_value_set_string (&fmt, "NV12");
    } else {
      g_value_set_string (&fmt, "BGR");
    }
    gst_value_list_append_value (&fmts, &fmt);
  } else {
    g_value_set_string (&fmt, "NV12");
    gst_value_list_append_value (&fmts, &fmt);
    g_value_set_string (&fmt, "BGR");
    gst_value_list_append_value (&fmts, &fmt);
  }

  if (gst_value_list_get_size (&fmts) != 0) {
    gst_structure_take_value (s, "format", &fmts);
    ret = TRUE;
  } else {
    g_value_unset (&fmts);
  }

  g_value_unset (&fmt);

  return ret;
}

static gboolean
load_venc_libraries (GstAmlVEnc * encoder)
{
  const gchar *libpath = AMLH264_ENCODER_LIBRARIES;
  if (encoder->enc_type == CODEC_ID_H265)
  {
    libpath = AMLH265_ENCODER_LIBRARIES;
  }
  gchar **libraries = g_strsplit (libpath, ":", -1);
  gchar **p = libraries;

  while (*p && !vtable_aml) {
    GstAmlVEncVTable *vtable = load_v (*p);

    if (vtable) {
      if (!vtable_aml) {
        vtable_aml = vtable;
      } else {
        unload_v (vtable);
      }
    }

    p++;
  }
  g_strfreev (libraries);

  if (!vtable_aml)
    return FALSE;

  vtable_aml->codec_id = encoder->enc_type;

  return TRUE;
}

#define PROP_IDR_PERIOD_DEFAULT 30
#define PROP_FRAMERATE_DEFAULT 30
#define PROP_BITRATE_DEFAULT 2000
#define PROP_BITRATE_MAX 12000
#define PROP_MIN_BUFFERS_DEFAULT 4
#define PROP_MAX_BUFFERS_DEFAULT 6
#define PROP_ENCODER_BUFFER_SIZE_DEFAULT 2048
#define PROP_ENCODER_BUFFER_SIZE_MIN 1024
#define PROP_ENCODER_BUFFER_SIZE_MAX 4096

enum
{
  PROP_0,
  PROP_GOP,
  PROP_FRAMERATE,
  PROP_BITRATE,
  PROP_MIN_BUFFERS,
  PROP_MAX_BUFFERS,
  PROP_ENCODER_BUFSIZE,
};

#define COMMON_SRC_PADS \
        "framerate = (fraction) [0/1, MAX], " \
        "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ], " \
        "stream-format = (string) { byte-stream }, " \
        "alignment = (string) au, " \
        "profile = (string) { high-4:4:4, high-4:2:2, high-10, high, main," \
        " baseline, constrained-baseline, high-4:4:4-intra, high-4:2:2-intra," \
        " high-10-intra }"

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264, "
        COMMON_SRC_PADS "; "
        "video/x-h265, "
        COMMON_SRC_PADS)
    );

static void gst_amlvenc_finalize (GObject * object);
static gboolean gst_amlvenc_start (GstVideoEncoder * encoder);
static gboolean gst_amlvenc_stop (GstVideoEncoder * encoder);
static gboolean gst_amlvenc_flush (GstVideoEncoder * encoder);

static gboolean gst_amlvenc_init_encoder (GstAmlVEnc * encoder);
static void gst_amlvenc_close_encoder (GstAmlVEnc * encoder);

static GstFlowReturn gst_amlvenc_finish (GstVideoEncoder * encoder);
static GstFlowReturn gst_amlvenc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame);
static void gst_amlvenc_flush_frames (GstAmlVEnc * encoder, gboolean send);
static GstFlowReturn gst_amlvenc_encode_frame (GstAmlVEnc * encoder,
    GstVideoCodecFrame * frame);
static gboolean gst_amlvenc_set_format (GstVideoEncoder * video_enc,
    GstVideoCodecState * state);
static gboolean gst_amlvenc_propose_allocation (GstVideoEncoder * encoder,
    GstQuery * query);

static void gst_amlvenc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_amlvenc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

#define gst_amlvenc_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstAmlVEnc, gst_amlvenc, GST_TYPE_VIDEO_ENCODER,
    G_IMPLEMENT_INTERFACE (GST_TYPE_PRESET, NULL));


/* allowed input caps depending on whether libv was built for 8 or 10 bits */
static GstCaps *
gst_amlvenc_sink_getcaps (GstVideoEncoder * enc, GstCaps * filter)
{
  GstCaps *supported_incaps;
  GstCaps *allowed;
  GstCaps *filter_caps, *fcaps;
  gint i, j;

  supported_incaps =
      gst_pad_get_pad_template_caps (GST_VIDEO_ENCODER_SINK_PAD (enc));

  /* Allow downstream to specify width/height/framerate/PAR constraints
   * and forward them upstream for video converters to handle
   */
  allowed = gst_pad_get_allowed_caps (enc->srcpad);

  if (!allowed || gst_caps_is_empty (allowed) || gst_caps_is_any (allowed)) {
    fcaps = supported_incaps;
    goto done;
  }

  GST_LOG_OBJECT (enc, "template caps %" GST_PTR_FORMAT, supported_incaps);
  GST_LOG_OBJECT (enc, "allowed caps %" GST_PTR_FORMAT, allowed);

  filter_caps = gst_caps_new_empty ();

  for (i = 0; i < gst_caps_get_size (supported_incaps); i++) {
    GQuark q_name =
        gst_structure_get_name_id (gst_caps_get_structure (supported_incaps,
            i));

    for (j = 0; j < gst_caps_get_size (allowed); j++) {
      const GstStructure *allowed_s = gst_caps_get_structure (allowed, j);
      const GValue *val;
      GstStructure *s;
      const gchar* allowed_mime_name = gst_structure_get_name (allowed_s);
      GstAmlVEnc *venc = GST_AMLVENC (enc);

      if (!g_strcmp0 (allowed_mime_name, "video/x-h265"))
      {
        venc->enc_type = CODEC_ID_H265;
      } else if (!g_strcmp0 (allowed_mime_name, "video/x-h264")) {
        venc->enc_type = CODEC_ID_H264;
      }

      if (vtable_aml == NULL)
      {
        if (!load_venc_libraries (venc))
          return NULL;
      }

      s = gst_structure_new_id_empty (q_name);
      if ((val = gst_structure_get_value (allowed_s, "width")))
        gst_structure_set_value (s, "width", val);
      if ((val = gst_structure_get_value (allowed_s, "height")))
        gst_structure_set_value (s, "height", val);
      if ((val = gst_structure_get_value (allowed_s, "framerate")))
        gst_structure_set_value (s, "framerate", val);
      if ((val = gst_structure_get_value (allowed_s, "pixel-aspect-ratio")))
        gst_structure_set_value (s, "pixel-aspect-ratio", val);

      gst_amlvenc_add_v_chroma_format (s);

      filter_caps = gst_caps_merge_structure (filter_caps, s);
    }
  }

  fcaps = gst_caps_intersect (filter_caps, supported_incaps);
  gst_caps_unref (filter_caps);
  gst_caps_unref (supported_incaps);

  if (filter) {
    GST_LOG_OBJECT (enc, "intersecting with %" GST_PTR_FORMAT, filter);
    filter_caps = gst_caps_intersect (fcaps, filter);
    gst_caps_unref (fcaps);
    fcaps = filter_caps;
  }

done:
  gst_caps_replace (&allowed, NULL);

  GST_LOG_OBJECT (enc, "proxy caps %" GST_PTR_FORMAT, fcaps);

  return fcaps;
}

static gboolean
gst_amlvenc_sink_query (GstVideoEncoder * enc, GstQuery * query)
{
  GstPad *pad = GST_VIDEO_ENCODER_SINK_PAD (enc);
  gboolean ret = FALSE;

  GST_DEBUG ("Received %s query on sinkpad, %" GST_PTR_FORMAT,
      GST_QUERY_TYPE_NAME (query), query);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_ACCEPT_CAPS:{
      GstCaps *acceptable, *caps;

      acceptable = gst_pad_get_pad_template_caps (pad);

      gst_query_parse_accept_caps (query, &caps);

      gst_query_set_accept_caps_result (query,
          gst_caps_is_subset (caps, acceptable));
      gst_caps_unref (acceptable);
      ret = TRUE;
    }
      break;
    default:
      ret = GST_VIDEO_ENCODER_CLASS (parent_class)->sink_query (enc, query);
      break;
  }

  return ret;
}

static void
gst_amlvenc_class_init (GstAmlVEncClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;
  GstVideoEncoderClass *gstencoder_class;
  GstPadTemplate *sink_templ;
  GstCaps *supported_sinkcaps;

  gobject_class = G_OBJECT_CLASS (klass);
  element_class = GST_ELEMENT_CLASS (klass);
  gstencoder_class = GST_VIDEO_ENCODER_CLASS (klass);

  gobject_class->set_property = gst_amlvenc_set_property;
  gobject_class->get_property = gst_amlvenc_get_property;
  gobject_class->finalize = gst_amlvenc_finalize;

  gstencoder_class->set_format = GST_DEBUG_FUNCPTR (gst_amlvenc_set_format);
  gstencoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_amlvenc_handle_frame);
  gstencoder_class->start = GST_DEBUG_FUNCPTR (gst_amlvenc_start);
  gstencoder_class->stop = GST_DEBUG_FUNCPTR (gst_amlvenc_stop);
  gstencoder_class->flush = GST_DEBUG_FUNCPTR (gst_amlvenc_flush);
  gstencoder_class->finish = GST_DEBUG_FUNCPTR (gst_amlvenc_finish);
  gstencoder_class->getcaps = GST_DEBUG_FUNCPTR (gst_amlvenc_sink_getcaps);
  gstencoder_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_amlvenc_propose_allocation);
  gstencoder_class->sink_query = GST_DEBUG_FUNCPTR (gst_amlvenc_sink_query);

  g_object_class_install_property (gobject_class, PROP_GOP,
      g_param_spec_int ("gop", "GOP", "IDR frame refresh interval",
          -1, 1000, PROP_IDR_PERIOD_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_FRAMERATE,
      g_param_spec_int ("framerate", "Framerate", "framerate(fps)",
          0, 30, PROP_FRAMERATE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BITRATE,
      g_param_spec_int ("bitrate", "Bitrate", "bitrate(bps)",
          0, PROP_BITRATE_MAX, PROP_BITRATE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_MIN_BUFFERS,
      g_param_spec_int ("min-buffers", "Min-Buffers", "min number of input buffer",
          3, 10, PROP_MIN_BUFFERS_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_MAX_BUFFERS,
      g_param_spec_int ("max-buffers", "Max-Buffers", "max number of input buffer",
          3, 10, PROP_MAX_BUFFERS_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ENCODER_BUFSIZE,
      g_param_spec_int ("encoder-buffer-size", "Encoder-Buffer-Size", "Encoder Buffer Size(KBytes)",
          PROP_ENCODER_BUFFER_SIZE_MIN, PROP_ENCODER_BUFFER_SIZE_MAX, PROP_ENCODER_BUFFER_SIZE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (element_class,
    "Amlogic h264/h265 Encoder",
    "Codec/Encoder/Video",
    "Amlogic h264/h265 Encoder Plugin",
    "Jemy Zhang <jun.zhang@amlogic.com>");

  supported_sinkcaps = gst_caps_new_simple ("video/x-raw",
      "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1,
      "width", GST_TYPE_INT_RANGE, 16, G_MAXINT,
      "height", GST_TYPE_INT_RANGE, 16, G_MAXINT, NULL);

  gst_amlvenc_add_v_chroma_format (gst_caps_get_structure (supported_sinkcaps, 0));

  sink_templ = gst_pad_template_new ("sink",
      GST_PAD_SINK, GST_PAD_ALWAYS, supported_sinkcaps);

  gst_caps_unref (supported_sinkcaps);

  gst_element_class_add_pad_template (element_class, sink_templ);
  gst_element_class_add_static_pad_template (element_class, &src_factory);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_amlvenc_init (GstAmlVEnc * encoder)
{
  encoder->gop = PROP_IDR_PERIOD_DEFAULT;
  encoder->framerate = PROP_FRAMERATE_DEFAULT;
  encoder->bitrate = PROP_BITRATE_DEFAULT;
  encoder->max_buffers = PROP_MAX_BUFFERS_DEFAULT;
  encoder->min_buffers = PROP_MIN_BUFFERS_DEFAULT;
  encoder->encoder_bufsize = PROP_ENCODER_BUFFER_SIZE_DEFAULT * 1024;
  encoder->enc_type = CODEC_ID_NONE;
}

static gboolean
gst_amlvenc_start (GstVideoEncoder * encoder)
{
  GstAmlVEnc *venc = GST_AMLVENC (encoder);

  if (venc->enc_buf == NULL)
  {
    venc->enc_buf = (gchar *) g_malloc (venc->encoder_bufsize);
  }
  /* make sure that we have enough time for first DTS,
     this is probably overkill for most streams */
  gst_video_encoder_set_min_pts (encoder, GST_MSECOND * 30);

  return TRUE;
}

static gboolean
gst_amlvenc_stop (GstVideoEncoder * encoder)
{
  GstAmlVEnc *venc = GST_AMLVENC (encoder);

  gst_amlvenc_flush_frames (venc, FALSE);
  gst_amlvenc_close_encoder (venc);

  if (venc->input_state)
    gst_video_codec_state_unref (venc->input_state);
  venc->input_state = NULL;

  if (venc->enc_buf)
  {
    g_free((gpointer)venc->enc_buf);
    venc->enc_buf = NULL;
  }

  return TRUE;
}


static gboolean
gst_amlvenc_flush (GstVideoEncoder * encoder)
{
  GstAmlVEnc *venc = GST_AMLVENC (encoder);

  gst_amlvenc_flush_frames (venc, FALSE);
  gst_amlvenc_close_encoder (venc);

  gst_amlvenc_init_encoder (venc);

  return TRUE;
}

static void
gst_amlvenc_finalize (GObject * object)
{
  GstAmlVEnc *encoder = GST_AMLVENC (object);

  if (encoder->input_state)
    gst_video_codec_state_unref (encoder->input_state);
  encoder->input_state = NULL;

  gst_amlvenc_close_encoder (encoder);

  if (vtable_aml) {
    unload_v (vtable_aml);
    vtable_aml = NULL;
  }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*
 * gst_amlvenc_init_encoder
 * @encoder:  Encoder which should be initialized.
 *
 * Initialize v encoder.
 *
 */
static gboolean
gst_amlvenc_init_encoder (GstAmlVEnc * encoder)
{
  GstVideoInfo *info;
  guint encoder_bitrate = encoder->bitrate * 1000;

  if (!encoder->input_state) {
    GST_DEBUG_OBJECT (encoder, "Have no input state yet");
    return FALSE;
  }

  info = &encoder->input_state->info;

  /* make sure that the encoder is closed */
  gst_amlvenc_close_encoder (encoder);

  GST_OBJECT_LOCK (encoder);

  encoder->vtable = vtable_aml;

  g_assert (encoder->vtable != NULL);

  GST_OBJECT_UNLOCK (encoder);

  encoder->venc = encoder->vtable->vl_video_encoder_init (encoder->enc_type,
      info->width, info->height,
      encoder->framerate, encoder_bitrate, encoder->gop,
      0);
  if (!encoder->venc) {
    GST_ELEMENT_ERROR (encoder, STREAM, ENCODE,
        ("Can not initialize v encoder."), (NULL));
    return FALSE;
  }

  return TRUE;
}

/* gst_amlvenc_close_encoder
 * @encoder:  Encoder which should close.
 *
 * Close v encoder.
 */
static void
gst_amlvenc_close_encoder (GstAmlVEnc * encoder)
{
  if (encoder->venc != 0) {
    encoder->vtable->vl_video_encoder_destory (encoder->venc);
    encoder->venc = 0;
  }
  encoder->vtable = 0;
}

static gboolean
gst_amlvenc_set_profile_and_level (GstAmlVEnc * encoder, GstCaps * caps)
{
  GstStructure *s;
  const gchar *profile;
  GstCaps *allowed_caps;
  GstStructure *s2;
  const gchar *allowed_profile;

  /* Constrained baseline is a strict subset of baseline. If downstream
   * wanted baseline and we produced constrained baseline, we can just
   * set the profile to baseline in the caps to make negotiation happy.
   * Same goes for baseline as subset of main profile and main as a subset
   * of high profile.
   */
  s = gst_caps_get_structure (caps, 0);
  profile = gst_structure_get_string (s, "profile");

  allowed_caps = gst_pad_get_allowed_caps (GST_VIDEO_ENCODER_SRC_PAD (encoder));

  if (allowed_caps == NULL)
    goto no_peer;

  if (!gst_caps_can_intersect (allowed_caps, caps)) {
    allowed_caps = gst_caps_make_writable (allowed_caps);
    allowed_caps = gst_caps_truncate (allowed_caps);
    s2 = gst_caps_get_structure (allowed_caps, 0);
    gst_structure_fixate_field_string (s2, "profile", profile);
    allowed_profile = gst_structure_get_string (s2, "profile");
    if (!g_strcmp0 (allowed_profile, "high")) {
      if (!g_strcmp0 (profile, "constrained-baseline")
          || !g_strcmp0 (profile, "baseline") || !g_strcmp0 (profile, "main")) {
        gst_structure_set (s, "profile", G_TYPE_STRING, "high", NULL);
        GST_INFO_OBJECT (encoder, "downstream requested high profile, but "
            "encoder will now output %s profile (which is a subset), due "
            "to how it's been configured", profile);
      }
    } else if (!g_strcmp0 (allowed_profile, "main")) {
      if (!g_strcmp0 (profile, "constrained-baseline")
          || !g_strcmp0 (profile, "baseline")) {
        gst_structure_set (s, "profile", G_TYPE_STRING, "main", NULL);
        GST_INFO_OBJECT (encoder, "downstream requested main profile, but "
            "encoder will now output %s profile (which is a subset), due "
            "to how it's been configured", profile);
      }
    } else if (!g_strcmp0 (allowed_profile, "baseline")) {
      if (!g_strcmp0 (profile, "constrained-baseline"))
        gst_structure_set (s, "profile", G_TYPE_STRING, "baseline", NULL);
    }
  }
  gst_caps_unref (allowed_caps);

no_peer:

  return TRUE;
}

/* gst_amlvenc_set_src_caps
 * Returns: TRUE on success.
 */
static gboolean
gst_amlvenc_set_src_caps (GstAmlVEnc * encoder, GstCaps * caps)
{
  GstCaps *outcaps;
  GstStructure *structure;
  GstVideoCodecState *state;
  GstTagList *tags;
  const gchar* mime_str = "video/x-h264";

  if (encoder->enc_type == CODEC_ID_H265) {
    mime_str = "video/x-h265";
  }
  outcaps = gst_caps_new_empty_simple (mime_str);
  structure = gst_caps_get_structure (outcaps, 0);

  gst_structure_set (structure, "stream-format", G_TYPE_STRING, "byte-stream",
      NULL);
  gst_structure_set (structure, "alignment", G_TYPE_STRING, "au", NULL);

  if (!gst_amlvenc_set_profile_and_level (encoder, outcaps)) {
    gst_caps_unref (outcaps);
    return FALSE;
  }

  state = gst_video_encoder_set_output_state (GST_VIDEO_ENCODER (encoder),
      outcaps, encoder->input_state);
  GST_DEBUG_OBJECT (encoder, "output caps: %" GST_PTR_FORMAT, state->caps);

  gst_video_codec_state_unref (state);

  tags = gst_tag_list_new_empty ();
  gst_tag_list_add (tags, GST_TAG_MERGE_REPLACE, GST_TAG_ENCODER, "v",
      GST_TAG_MAXIMUM_BITRATE, encoder->bitrate * 1000,
      GST_TAG_NOMINAL_BITRATE, encoder->bitrate * 1000, NULL);
  gst_video_encoder_merge_tags (GST_VIDEO_ENCODER (encoder), tags,
      GST_TAG_MERGE_REPLACE);
  gst_tag_list_unref (tags);

  return TRUE;
}

static void
gst_amlvenc_set_latency (GstAmlVEnc * encoder)
{
  GstVideoInfo *info = &encoder->input_state->info;
  gint max_delayed_frames;
  GstClockTime latency;

  max_delayed_frames = 0;

  if (info->fps_n) {
    latency = gst_util_uint64_scale_ceil (GST_SECOND * info->fps_d,
        max_delayed_frames, info->fps_n);
  } else {
    /* FIXME: Assume 25fps. This is better than reporting no latency at
     * all and then later failing in live pipelines
     */
    latency = gst_util_uint64_scale_ceil (GST_SECOND * 1,
        max_delayed_frames, 25);
  }

  GST_INFO_OBJECT (encoder,
      "Updating latency to %" GST_TIME_FORMAT " (%d frames)",
      GST_TIME_ARGS (latency), max_delayed_frames);

  gst_video_encoder_set_latency (GST_VIDEO_ENCODER (encoder), latency, latency);
}

static gboolean
gst_amlvenc_set_format (GstVideoEncoder * video_enc,
    GstVideoCodecState * state)
{
  GstAmlVEnc *encoder = GST_AMLVENC (video_enc);
  GstVideoInfo *info = &state->info;
  GstCaps *template_caps;
  GstCaps *allowed_caps = NULL;
  const gchar* allowed_mime_name = NULL;

  /* If the encoder is initialized, do not reinitialize it again if not
   * necessary */
  if (encoder->venc) {
    GstVideoInfo *old = &encoder->input_state->info;

    if (info->finfo->format == old->finfo->format
        && info->width == old->width && info->height == old->height
        && info->fps_n == old->fps_n && info->fps_d == old->fps_d
        && info->par_n == old->par_n && info->par_d == old->par_d) {
      gst_video_codec_state_unref (encoder->input_state);
      encoder->input_state = gst_video_codec_state_ref (state);
      return TRUE;
    }

    /* clear out pending frames */
    gst_amlvenc_flush_frames (encoder, TRUE);
  }

  if (encoder->input_state)
    gst_video_codec_state_unref (encoder->input_state);
  encoder->input_state = gst_video_codec_state_ref (state);

  template_caps = gst_static_pad_template_get_caps (&src_factory);
  allowed_caps = gst_pad_get_allowed_caps (GST_VIDEO_ENCODER_SRC_PAD (encoder));

  if (allowed_caps && allowed_caps != template_caps && encoder->enc_type == CODEC_ID_NONE) {
    GstStructure *s;

    if (gst_caps_is_empty (allowed_caps)) {
      gst_caps_unref (allowed_caps);
      gst_caps_unref (template_caps);
      return FALSE;
    }

    allowed_caps = gst_caps_make_writable (allowed_caps);
    allowed_caps = gst_caps_fixate (allowed_caps);
    s = gst_caps_get_structure (allowed_caps, 0);
    allowed_mime_name = gst_structure_get_name (s);

    if (!g_strcmp0 (allowed_mime_name, "video/x-h265"))
    {
      encoder->enc_type = CODEC_ID_H265;
    } else {
      encoder->enc_type = CODEC_ID_H264;
    }

    gst_caps_unref (allowed_caps);
  }

  gst_caps_unref (template_caps);

  if (!gst_amlvenc_init_encoder (encoder))
    return FALSE;

  if (!gst_amlvenc_set_src_caps (encoder, state->caps)) {
    gst_amlvenc_close_encoder (encoder);
    return FALSE;
  }

  gst_amlvenc_set_latency (encoder);

  return TRUE;
}

static GstFlowReturn
gst_amlvenc_finish (GstVideoEncoder * encoder)
{
  gst_amlvenc_flush_frames (GST_AMLVENC (encoder), TRUE);
  return GST_FLOW_OK;
}

static gboolean
gst_amlvenc_propose_allocation (GstVideoEncoder * encoder, GstQuery * query)
{
  GstAmlVEnc *self = GST_AMLVENC (encoder);
  GstVideoInfo *info;
  guint size, min = 0, max = 0;

  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);

  if (!self->input_state)
    return FALSE;

  if (self->vtable == NULL)
    return FALSE;

  info = &self->input_state->info;
  if (gst_query_get_n_allocation_pools (query) > 0) {
    gst_query_parse_nth_allocation_pool (query, 0, NULL, &size, &min, &max);
    size = MAX (size, info->size);
    gst_query_set_nth_allocation_pool (query, 0, NULL, size, self->min_buffers, self->max_buffers);
  } else {
    gst_query_add_allocation_pool (query, NULL, info->size, self->min_buffers, self->max_buffers);
  }

  return GST_VIDEO_ENCODER_CLASS (parent_class)->propose_allocation (encoder,
      query);
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_amlvenc_handle_frame (GstVideoEncoder * video_enc,
    GstVideoCodecFrame * frame)
{
  GstAmlVEnc *encoder = GST_AMLVENC (video_enc);
  GstFlowReturn ret;

  if (G_UNLIKELY (encoder->venc == 0))
    goto not_inited;

  ret = gst_amlvenc_encode_frame (encoder, frame);

  /* input buffer is released later on */
  return ret;

/* ERRORS */
not_inited:
  {
    GST_WARNING_OBJECT (encoder, "Got buffer before set_caps was called");
    return GST_FLOW_NOT_NEGOTIATED;
  }
}

#include <time.h>

static GstFlowReturn
gst_amlvenc_encode_frame (GstAmlVEnc * encoder,
    GstVideoCodecFrame * frame)
{
  GstVideoFrame video_frame;
  vl_frame_type_t frame_type = FRAME_TYPE_AUTO;
  GstVideoInfo *info = &encoder->input_state->info;
  GstMapInfo map;
  gint encode_data_len = -1;

  if (G_UNLIKELY (encoder->venc == 0)) {
    if (frame)
      gst_video_codec_frame_unref (frame);
    return GST_FLOW_NOT_NEGOTIATED;
  }

  if (frame) {
    gst_video_frame_map(&video_frame, info, frame->input_buffer, GST_MAP_READ);
    if (GST_VIDEO_CODEC_FRAME_IS_FORCE_KEYFRAME (frame)) {
      GST_INFO_OBJECT (encoder, "Forcing key frame");
      frame_type = FRAME_TYPE_IDR;
    }
  }

  GstVideoFormat vfmt = info->finfo->format;
  int fmt = 0;
  if (encoder->enc_type == CODEC_ID_H265) {
    switch (vfmt) {
      case GST_VIDEO_FORMAT_NV21:
        fmt = 0;
        break;
      case GST_VIDEO_FORMAT_NV12:
        fmt = 1;
        break;
      case GST_VIDEO_FORMAT_RGB:
        fmt = 2;
        break;
      case GST_VIDEO_FORMAT_I420:
        fmt = 3;
        break;
      default:
        fmt = 3;
        break;
    }
  } else {
    switch (vfmt) {
      case GST_VIDEO_FORMAT_NV21:
        fmt = 0;
        break;
      case GST_VIDEO_FORMAT_I420:
        fmt = 1;
        break;
      case GST_VIDEO_FORMAT_RGB:
        fmt = 2;
        break;
      case GST_VIDEO_FORMAT_BGR:
        fmt = 3;
        break;
      default:
        fmt = 1;
        break;
    }
  }

  encode_data_len = encoder->vtable->vl_video_encoder_encode (encoder->venc,
      frame_type, GST_VIDEO_FRAME_PLANE_DATA (&video_frame, 0), encoder->encoder_bufsize, encoder->enc_buf, fmt);

  if (encode_data_len < 0) {
    if (frame) {
      GST_ELEMENT_ERROR (encoder, STREAM, ENCODE, ("Encode v frame failed."),
          ("gst_amlvencoder_encode return code=%d", encode_data_len));
      gst_video_frame_unmap (&video_frame);
      gst_video_codec_frame_unref (frame);
      return GST_FLOW_ERROR;
    } else {
      return GST_FLOW_EOS;
    }
  }

  if (frame) {
    gst_video_frame_unmap (&video_frame);
    gst_video_codec_frame_unref (frame);
  }

  //frame = gst_video_encoder_get_frame (GST_VIDEO_ENCODER (encoder), input_frame->system_frame_number);
  frame = gst_video_encoder_get_oldest_frame (GST_VIDEO_ENCODER (encoder));
  if (!frame) {
    gst_video_codec_frame_unref (frame);
    return GST_FLOW_ERROR;
  }

  frame->output_buffer = gst_video_encoder_allocate_output_buffer (GST_VIDEO_ENCODER (encoder), encode_data_len);
  gst_buffer_map (frame->output_buffer, &map, GST_MAP_WRITE);
  g_memmove (map.data, encoder->enc_buf, encode_data_len);
  gst_buffer_unmap (frame->output_buffer, &map);

  GST_LOG_OBJECT (encoder,
      "output: dts %" G_GINT64_FORMAT " pts %" G_GINT64_FORMAT,
      (gint64) frame->dts, (gint64) frame->pts);

  if (frame_type == FRAME_TYPE_IDR) {
    GST_DEBUG_OBJECT (encoder, "Output keyframe");
    GST_VIDEO_CODEC_FRAME_SET_SYNC_POINT (frame);
  } else {
    GST_VIDEO_CODEC_FRAME_UNSET_SYNC_POINT (frame);
  }

  return gst_video_encoder_finish_frame ( GST_VIDEO_ENCODER(encoder), frame);

}

static void
gst_amlvenc_flush_frames (GstAmlVEnc * encoder, gboolean send)
{
}

static void
gst_amlvenc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAmlVEnc *encoder = GST_AMLVENC (object);

  GST_OBJECT_LOCK (encoder);
  switch (prop_id) {
    case PROP_GOP:
      g_value_set_int (value, encoder->gop);
      break;
    case PROP_FRAMERATE:
      g_value_set_int (value, encoder->framerate);
      break;
    case PROP_BITRATE:
      g_value_set_int (value, encoder->bitrate);
      break;
    case PROP_MIN_BUFFERS:
      g_value_set_int (value, encoder->min_buffers);
      break;
    case PROP_MAX_BUFFERS:
      g_value_set_int (value, encoder->max_buffers);
      break;
    case PROP_ENCODER_BUFSIZE:
      g_value_set_int (value, encoder->encoder_bufsize / 1024);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (encoder);
}

static void
gst_amlvenc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmlVEnc *encoder = GST_AMLVENC (object);
  GstState state;

  GST_OBJECT_LOCK (encoder);

  state = GST_STATE (encoder);

  if ((state != GST_STATE_READY && state != GST_STATE_NULL) &&
      !(pspec->flags & GST_PARAM_MUTABLE_PLAYING))
  {
    GST_WARNING_OBJECT (encoder, "setting property in wrong state");
    GST_OBJECT_UNLOCK (encoder);
    return;
  }

  switch (prop_id) {
    case PROP_GOP:
      encoder->gop = g_value_get_int (value);
      break;
    case PROP_FRAMERATE:
      encoder->framerate = g_value_get_int (value);
      break;
    case PROP_BITRATE:
      encoder->bitrate = g_value_get_int (value);
      break;
    case PROP_MIN_BUFFERS:
      encoder->min_buffers = g_value_get_int (value);
      break;
    case PROP_MAX_BUFFERS:
      encoder->max_buffers = g_value_get_int (value);
      break;
    case PROP_ENCODER_BUFSIZE:
      encoder->encoder_bufsize = g_value_get_int (value) * 1024;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

  GST_OBJECT_UNLOCK (encoder);
  return;
}

static gboolean
amlvenc_init (GstPlugin * amlvenc)
{
  GST_DEBUG_CATEGORY_INIT (gst_amlvenc_debug, "amlvenc", 0,
      "amlogic h264/h265 encoding element");

  return gst_element_register (amlvenc, "amlvenc", GST_RANK_PRIMARY,
      GST_TYPE_AMLVENC);
}

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amlvenc,
    "Amlogic h264/h265 encoder plugins",
    amlvenc_init,
    VERSION,
    "LGPL",
    "amlogic h264/h265 ecoding",
    "http://openlinux.amlogic.com"
)
