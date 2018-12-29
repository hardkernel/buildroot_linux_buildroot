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
 * SECTION:element-amlx265enc
 *
 * FIXME:Describe amlx265enc here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! amlx265enc ! fakesink silent=TRUE
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

#include "gstamlx265enc.h"

GST_DEBUG_CATEGORY_STATIC (gst_amlx265enc_debug);
#define GST_CAT_DEFAULT gst_amlx265enc_debug

#define HAVE_AMLX265_ADDITIONAL_LIBRARIES "/usr/lib/libvphevcodec.so"


typedef enum vl_codec_id_e {
  CODEC_ID_NONE,
  CODEC_ID_VP8,
  CODEC_ID_H261,
  CODEC_ID_H263,
  CODEC_ID_H264,
  CODEC_ID_H265, /* must support */

} vl_codec_id_t;

typedef enum vl_img_format_e
{
  IMG_FMT_NONE,
  IMG_FMT_NV12, /* must support  */
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


struct _GstAmlX265EncVTable
{
  GModule *module;

  vl_codec_handle_t (*vl_video_encoder_init)(vl_codec_id_t codec_id, int width, int height, int frame_rate, int bit_rate, int gop, vl_img_format_t img_format);
  int (*vl_video_encoder_encode)(vl_codec_handle_t codec_handle, int frame_type, unsigned char *in, int in_size, unsigned char *out, int format);
  int (*vl_video_encoder_destory)(vl_codec_handle_t codec_handle);
};

static GstAmlX265EncVTable default_vtable;

static GstAmlX265EncVTable *vtable_aml = NULL;

#define LOAD_SYMBOL(name) G_STMT_START { \
  if (!g_module_symbol (module, #name, (gpointer *) &vtable->name)) { \
    GST_ERROR ("Failed to load '" #name "' from '%s'", filename); \
    goto error; \
  } \
} G_STMT_END;

static GstAmlX265EncVTable *
load_x265 (const gchar * filename)
{
  GModule *module;
  GstAmlX265EncVTable *vtable;

  module = g_module_open (filename, G_MODULE_BIND_LOCAL);
  if (!module) {
    GST_ERROR ("Failed to load '%s'", filename);
    return NULL;
  }

  vtable = g_new0 (GstAmlX265EncVTable, 1);
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
unload_x265 (GstAmlX265EncVTable * vtable)
{
  if (vtable->module) {
    g_module_close (vtable->module);
    g_free (vtable);
  }
}

#undef LOAD_SYMBOL

static gboolean
gst_amlx265enc_add_x265_chroma_format (GstStructure * s,
    gboolean allow_420, gboolean allow_422, gboolean allow_444)
{
  GValue fmts = G_VALUE_INIT;
  GValue fmt = G_VALUE_INIT;
  gboolean ret = FALSE;

  g_value_init (&fmts, GST_TYPE_LIST);
  g_value_init (&fmt, G_TYPE_STRING);

  if (vtable_aml) {
    g_value_set_string (&fmt, "I420");
    gst_value_list_append_value (&fmts, &fmt);
    g_value_set_string (&fmt, "NV21");
    gst_value_list_append_value (&fmts, &fmt);
    g_value_set_string (&fmt, "RGB");
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
load_x265_libraries (void)
{
  gchar **libraries = g_strsplit (HAVE_AMLX265_ADDITIONAL_LIBRARIES, ":", -1);
  gchar **p = libraries;

  while (*p && !vtable_aml) {
    GstAmlX265EncVTable *vtable = load_x265 (*p);

    if (vtable) {
      if (!vtable_aml) {
        vtable_aml = vtable;
      } else {
        unload_x265 (vtable);
      }
    }

    p++;
  }
  g_strfreev (libraries);

  if (!vtable_aml)
    return FALSE;

  return TRUE;
}

#define PROP_IDR_PERIOD_DEFAULT 30
#define PROP_FRAMERATE_DEFAULT 30
#define PROP_BITRATE_DEFAULT 2000
#define PROP_BITRATE_MAX 6000
#define PROP_MIN_BUFFERS_DEFAULT 4
#define PROP_MAX_BUFFERS_DEFAULT 6

enum
{
  PROP_0,
  PROP_GOP,
  PROP_FRAMERATE,
  PROP_BITRATE,
  PROP_MIN_BUFFERS,
  PROP_MAX_BUFFERS,
};

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h265, "
        "framerate = (fraction) [0/1, MAX], "
        "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ], "
        "stream-format = (string) { byte-stream }, "
        "alignment = (string) au, "
        "profile = (string) { high-4:4:4, high-4:2:2, high-10, high, main,"
        " baseline, constrained-baseline, high-4:4:4-intra, high-4:2:2-intra,"
        " high-10-intra }")
    );

static void gst_amlx265enc_finalize (GObject * object);
static gboolean gst_amlx265enc_start (GstVideoEncoder * encoder);
static gboolean gst_amlx265enc_stop (GstVideoEncoder * encoder);
static gboolean gst_amlx265enc_flush (GstVideoEncoder * encoder);

static gboolean gst_amlx265enc_init_encoder (GstAmlX265Enc * encoder);
static void gst_amlx265enc_close_encoder (GstAmlX265Enc * encoder);

static GstFlowReturn gst_amlx265enc_finish (GstVideoEncoder * encoder);
static GstFlowReturn gst_amlx265enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame);
static void gst_amlx265enc_flush_frames (GstAmlX265Enc * encoder, gboolean send);
static GstFlowReturn gst_amlx265enc_encode_frame (GstAmlX265Enc * encoder,
    GstVideoCodecFrame * input_frame,
    gboolean send);
static gboolean gst_amlx265enc_set_format (GstVideoEncoder * video_enc,
    GstVideoCodecState * state);
static gboolean gst_amlx265enc_propose_allocation (GstVideoEncoder * encoder,
    GstQuery * query);

static void gst_amlx265enc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_amlx265enc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

#define gst_amlx265enc_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstAmlX265Enc, gst_amlx265enc, GST_TYPE_VIDEO_ENCODER,
    G_IMPLEMENT_INTERFACE (GST_TYPE_PRESET, NULL));

static void
check_formats (const gchar * str, gboolean * has_420, gboolean * has_422,
    gboolean * has_444)
{
  if (g_str_has_prefix (str, "high-4:4:4"))
    *has_444 = TRUE;
  else if (g_str_has_prefix (str, "high-4:2:2"))
    *has_422 = TRUE;
  else
    *has_420 = TRUE;
}


/* allowed input caps depending on whether libx265 was built for 8 or 10 bits */
static GstCaps *
gst_amlx265enc_sink_getcaps (GstVideoEncoder * enc, GstCaps * filter)
{
  GstCaps *supported_incaps;
  GstCaps *allowed;
  GstCaps *filter_caps, *fcaps;
  gint i, j, k;

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

      s = gst_structure_new_id_empty (q_name);
      if ((val = gst_structure_get_value (allowed_s, "width")))
        gst_structure_set_value (s, "width", val);
      if ((val = gst_structure_get_value (allowed_s, "height")))
        gst_structure_set_value (s, "height", val);
      if ((val = gst_structure_get_value (allowed_s, "framerate")))
        gst_structure_set_value (s, "framerate", val);
      if ((val = gst_structure_get_value (allowed_s, "pixel-aspect-ratio")))
        gst_structure_set_value (s, "pixel-aspect-ratio", val);

      if ((val = gst_structure_get_value (allowed_s, "profile"))) {
        gboolean has_420 = FALSE;
        gboolean has_422 = FALSE;
        gboolean has_444 = FALSE;

        if (G_VALUE_HOLDS_STRING (val)) {
          check_formats (g_value_get_string (val), &has_420, &has_422,
              &has_444);
        } else if (GST_VALUE_HOLDS_LIST (val)) {
          for (k = 0; k < gst_value_list_get_size (val); k++) {
            const GValue *vlist = gst_value_list_get_value (val, k);

            if (G_VALUE_HOLDS_STRING (vlist))
              check_formats (g_value_get_string (vlist), &has_420, &has_422,
                  &has_444);
          }
        }

        gst_amlx265enc_add_x265_chroma_format (s, has_420, has_422, has_444);
      }

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
gst_amlx265enc_sink_query (GstVideoEncoder * enc, GstQuery * query)
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
gst_amlx265enc_class_init (GstAmlX265EncClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;
  GstVideoEncoderClass *gstencoder_class;
  GstPadTemplate *sink_templ;
  GstCaps *supported_sinkcaps;

  gobject_class = G_OBJECT_CLASS (klass);
  element_class = GST_ELEMENT_CLASS (klass);
  gstencoder_class = GST_VIDEO_ENCODER_CLASS (klass);

  gobject_class->set_property = gst_amlx265enc_set_property;
  gobject_class->get_property = gst_amlx265enc_get_property;
  gobject_class->finalize = gst_amlx265enc_finalize;

  gstencoder_class->set_format = GST_DEBUG_FUNCPTR (gst_amlx265enc_set_format);
  gstencoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_amlx265enc_handle_frame);
  gstencoder_class->start = GST_DEBUG_FUNCPTR (gst_amlx265enc_start);
  gstencoder_class->stop = GST_DEBUG_FUNCPTR (gst_amlx265enc_stop);
  gstencoder_class->flush = GST_DEBUG_FUNCPTR (gst_amlx265enc_flush);
  gstencoder_class->finish = GST_DEBUG_FUNCPTR (gst_amlx265enc_finish);
  gstencoder_class->getcaps = GST_DEBUG_FUNCPTR (gst_amlx265enc_sink_getcaps);
  gstencoder_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_amlx265enc_propose_allocation);
  gstencoder_class->sink_query = GST_DEBUG_FUNCPTR (gst_amlx265enc_sink_query);

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

  gst_element_class_set_static_metadata (element_class,
    "amlx265enc",
    "Codec/Encoder/Video",
    "H265 Encoder",
    "Jemy Zhang <jun.zhang@amlogic.com>");

  supported_sinkcaps = gst_caps_new_simple ("video/x-raw",
      "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1,
      "width", GST_TYPE_INT_RANGE, 16, G_MAXINT,
      "height", GST_TYPE_INT_RANGE, 16, G_MAXINT, NULL);

  gst_amlx265enc_add_x265_chroma_format (gst_caps_get_structure
      (supported_sinkcaps, 0), TRUE, TRUE, TRUE);

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
gst_amlx265enc_init (GstAmlX265Enc * encoder)
{
  encoder->gop = PROP_IDR_PERIOD_DEFAULT;
  encoder->framerate = PROP_FRAMERATE_DEFAULT;
  encoder->max_buffers = PROP_MAX_BUFFERS_DEFAULT;
  encoder->min_buffers = PROP_MIN_BUFFERS_DEFAULT;
}

typedef struct
{
  GstVideoCodecFrame *frame;
  GstVideoFrame vframe;
} FrameData;

static FrameData *
gst_amlx265enc_queue_frame (GstAmlX265Enc * enc, GstVideoCodecFrame * frame,
    GstVideoInfo * info)
{
  GstVideoFrame vframe;
  FrameData *fdata;

  if (!gst_video_frame_map (&vframe, info, frame->input_buffer, GST_MAP_READ))
    return NULL;

  fdata = g_slice_new (FrameData);
  fdata->frame = gst_video_codec_frame_ref (frame);
  fdata->vframe = vframe;

  enc->pending_frames = g_list_prepend (enc->pending_frames, fdata);

  return fdata;
}

static void
gst_amlx265enc_dequeue_frame (GstAmlX265Enc * enc, GstVideoCodecFrame * frame)
{
  GList *l;

  for (l = enc->pending_frames; l; l = l->next) {
    FrameData *fdata = l->data;

    if (fdata->frame != frame)
      continue;

    gst_video_frame_unmap (&fdata->vframe);
    gst_video_codec_frame_unref (fdata->frame);
    g_slice_free (FrameData, fdata);

    enc->pending_frames = g_list_delete_link (enc->pending_frames, l);
    return;
  }
}

static void
gst_amlx265enc_dequeue_all_frames (GstAmlX265Enc * enc)
{
  GList *l;

  for (l = enc->pending_frames; l; l = l->next) {
    FrameData *fdata = l->data;

    gst_video_frame_unmap (&fdata->vframe);
    gst_video_codec_frame_unref (fdata->frame);
    g_slice_free (FrameData, fdata);
  }
  g_list_free (enc->pending_frames);
  enc->pending_frames = NULL;
}

static gboolean
gst_amlx265enc_start (GstVideoEncoder * encoder)
{
  GstAmlX265Enc *x265enc = GST_AMLX265ENC (encoder);

  /* make sure that we have enough time for first DTS,
     this is probably overkill for most streams */
  gst_video_encoder_set_min_pts (encoder, GST_SECOND * 60 * 60 * 1000);

  return TRUE;
}

static gboolean
gst_amlx265enc_stop (GstVideoEncoder * encoder)
{
  GstAmlX265Enc *x265enc = GST_AMLX265ENC (encoder);

  gst_amlx265enc_flush_frames (x265enc, FALSE);
  gst_amlx265enc_close_encoder (x265enc);
  gst_amlx265enc_dequeue_all_frames (x265enc);

  if (x265enc->input_state)
    gst_video_codec_state_unref (x265enc->input_state);
  x265enc->input_state = NULL;

  return TRUE;
}


static gboolean
gst_amlx265enc_flush (GstVideoEncoder * encoder)
{
  GstAmlX265Enc *x265enc = GST_AMLX265ENC (encoder);

  gst_amlx265enc_flush_frames (x265enc, FALSE);
  gst_amlx265enc_close_encoder (x265enc);
  gst_amlx265enc_dequeue_all_frames (x265enc);

  gst_amlx265enc_init_encoder (x265enc);

  return TRUE;
}

static void
gst_amlx265enc_finalize (GObject * object)
{
  GstAmlX265Enc *encoder = GST_AMLX265ENC (object);

  if (encoder->input_state)
    gst_video_codec_state_unref (encoder->input_state);
  encoder->input_state = NULL;

  gst_amlx265enc_close_encoder (encoder);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*
 * gst_amlx265enc_init_encoder
 * @encoder:  Encoder which should be initialized.
 *
 * Initialize x265 encoder.
 *
 */
static gboolean
gst_amlx265enc_init_encoder (GstAmlX265Enc * encoder)
{
  GstVideoInfo *info;
  guint encoder_bitrate = encoder->bitrate * 1000;

  if (!encoder->input_state) {
    GST_DEBUG_OBJECT (encoder, "Have no input state yet");
    return FALSE;
  }

  info = &encoder->input_state->info;

  /* make sure that the encoder is closed */
  gst_amlx265enc_close_encoder (encoder);

  GST_OBJECT_LOCK (encoder);

  encoder->vtable = vtable_aml;

  g_assert (encoder->vtable != NULL);

  encoder->reconfig = FALSE;

  GST_OBJECT_UNLOCK (encoder);

  if (encoder->framerate < 20) {
    // adaptive bitrate on low framerate
    encoder_bitrate = encoder_bitrate * encoder->framerate / 30;
  }

  encoder->x265enc = encoder->vtable->vl_video_encoder_init (CODEC_ID_H265,
      info->width, info->height,
      encoder->framerate, encoder_bitrate, encoder->gop,
      0);
  if (!encoder->x265enc) {
    GST_ELEMENT_ERROR (encoder, STREAM, ENCODE,
        ("Can not initialize x265 encoder."), (NULL));
    return FALSE;
  }

  return TRUE;
}

/* gst_amlx265enc_close_encoder
 * @encoder:  Encoder which should close.
 *
 * Close x265 encoder.
 */
static void
gst_amlx265enc_close_encoder (GstAmlX265Enc * encoder)
{
  if (encoder->x265enc != 0) {
    encoder->vtable->vl_video_encoder_destory (encoder->x265enc);
    encoder->x265enc = 0;
  }
  encoder->vtable = 0;
}

static gboolean
gst_amlx265enc_set_profile_and_level (GstAmlX265Enc * encoder, GstCaps * caps)
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

/* gst_amlx265enc_set_src_caps
 * Returns: TRUE on success.
 */
static gboolean
gst_amlx265enc_set_src_caps (GstAmlX265Enc * encoder, GstCaps * caps)
{
  GstCaps *outcaps;
  GstStructure *structure;
  GstVideoCodecState *state;
  GstTagList *tags;

  outcaps = gst_caps_new_empty_simple ("video/x-h265");
  structure = gst_caps_get_structure (outcaps, 0);

  gst_structure_set (structure, "stream-format", G_TYPE_STRING, "byte-stream",
      NULL);
  gst_structure_set (structure, "alignment", G_TYPE_STRING, "au", NULL);

  if (!gst_amlx265enc_set_profile_and_level (encoder, outcaps)) {
    gst_caps_unref (outcaps);
    return FALSE;
  }

  state = gst_video_encoder_set_output_state (GST_VIDEO_ENCODER (encoder),
      outcaps, encoder->input_state);
  GST_DEBUG_OBJECT (encoder, "output caps: %" GST_PTR_FORMAT, state->caps);

  /* If set, local frame packing setting overrides any upstream config */
  switch (encoder->frame_packing) {
    case 0:
      GST_VIDEO_INFO_MULTIVIEW_MODE (&state->info) =
          GST_VIDEO_MULTIVIEW_MODE_CHECKERBOARD;
      break;
    case 1:
      GST_VIDEO_INFO_MULTIVIEW_MODE (&state->info) =
          GST_VIDEO_MULTIVIEW_MODE_COLUMN_INTERLEAVED;
      break;
    case 2:
      GST_VIDEO_INFO_MULTIVIEW_MODE (&state->info) =
          GST_VIDEO_MULTIVIEW_MODE_ROW_INTERLEAVED;
      break;
    case 3:
      GST_VIDEO_INFO_MULTIVIEW_MODE (&state->info) =
          GST_VIDEO_MULTIVIEW_MODE_SIDE_BY_SIDE;
      break;
    case 4:
      GST_VIDEO_INFO_MULTIVIEW_MODE (&state->info) =
          GST_VIDEO_MULTIVIEW_MODE_TOP_BOTTOM;
      break;
    case 5:
      GST_VIDEO_INFO_MULTIVIEW_MODE (&state->info) =
          GST_VIDEO_MULTIVIEW_MODE_FRAME_BY_FRAME;
      break;
    default:
      break;
  }

  gst_video_codec_state_unref (state);

  tags = gst_tag_list_new_empty ();
  gst_tag_list_add (tags, GST_TAG_MERGE_REPLACE, GST_TAG_ENCODER, "x265",
      GST_TAG_MAXIMUM_BITRATE, encoder->bitrate * 1000,
      GST_TAG_NOMINAL_BITRATE, encoder->bitrate * 1000, NULL);
  gst_video_encoder_merge_tags (GST_VIDEO_ENCODER (encoder), tags,
      GST_TAG_MERGE_REPLACE);
  gst_tag_list_unref (tags);

  return TRUE;
}

static void
gst_amlx265enc_set_latency (GstAmlX265Enc * encoder)
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
gst_amlx265enc_set_format (GstVideoEncoder * video_enc,
    GstVideoCodecState * state)
{
  GstAmlX265Enc *encoder = GST_AMLX265ENC (video_enc);
  GstVideoInfo *info = &state->info;
  GstCaps *template_caps;
  GstCaps *allowed_caps = NULL;

  /* If the encoder is initialized, do not reinitialize it again if not
   * necessary */
  if (encoder->x265enc) {
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
    gst_amlx265enc_flush_frames (encoder, TRUE);

    encoder->sps_id++;
  }

  if (encoder->input_state)
    gst_video_codec_state_unref (encoder->input_state);
  encoder->input_state = gst_video_codec_state_ref (state);

  encoder->peer_profile = NULL;
  encoder->peer_intra_profile = FALSE;
  encoder->peer_level_idc = -1;

  template_caps = gst_static_pad_template_get_caps (&src_factory);
  allowed_caps = gst_pad_get_allowed_caps (GST_VIDEO_ENCODER_SRC_PAD (encoder));

  if (allowed_caps && allowed_caps != template_caps) {
    GstStructure *s;
    const gchar *profile;
    const gchar *level;

    if (gst_caps_is_empty (allowed_caps)) {
      gst_caps_unref (allowed_caps);
      gst_caps_unref (template_caps);
      return FALSE;
    }

    allowed_caps = gst_caps_make_writable (allowed_caps);
    allowed_caps = gst_caps_fixate (allowed_caps);
    s = gst_caps_get_structure (allowed_caps, 0);

    profile = gst_structure_get_string (s, "profile");
    if (profile) {
      /* FIXME - if libx265 ever adds support for FMO, ASO or redundant slices
       * make sure constrained profile has a separate case which disables
       * those */
      if (g_str_has_suffix (profile, "-intra")) {
        encoder->peer_intra_profile = TRUE;
      }
      if (!g_strcmp0 (profile, "constrained-baseline") ||
          !g_strcmp0 (profile, "baseline")) {
        encoder->peer_profile = "baseline";
      } else if (g_str_has_prefix (profile, "high-10")) {
        encoder->peer_profile = "high10";
      } else if (g_str_has_prefix (profile, "high-4:2:2")) {
        encoder->peer_profile = "high422";
      } else if (g_str_has_prefix (profile, "high-4:4:4")) {
        encoder->peer_profile = "high444";
      } else if (g_str_has_prefix (profile, "high")) {
        encoder->peer_profile = "high";
      } else if (!g_strcmp0 (profile, "main")) {
        encoder->peer_profile = "main";
      } else {
        g_assert_not_reached ();
      }
    }

    level = gst_structure_get_string (s, "level");
    if (level) {
      encoder->peer_level_idc = gst_codec_utils_h265_get_level_idc (level);
    }

    gst_caps_unref (allowed_caps);
  }

  gst_caps_unref (template_caps);

  if (!gst_amlx265enc_init_encoder (encoder))
    return FALSE;

  if (!gst_amlx265enc_set_src_caps (encoder, state->caps)) {
    gst_amlx265enc_close_encoder (encoder);
    return FALSE;
  }

  gst_amlx265enc_set_latency (encoder);

  return TRUE;
}

static GstFlowReturn
gst_amlx265enc_finish (GstVideoEncoder * encoder)
{
  gst_amlx265enc_flush_frames (GST_AMLX265ENC (encoder), TRUE);
  return GST_FLOW_OK;
}

static gboolean
gst_amlx265enc_propose_allocation (GstVideoEncoder * encoder, GstQuery * query)
{
  GstAmlX265Enc *self = GST_AMLX265ENC (encoder);
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
gst_amlx265enc_handle_frame (GstVideoEncoder * video_enc,
    GstVideoCodecFrame * frame)
{
  GstAmlX265Enc *encoder = GST_AMLX265ENC (video_enc);
  GstVideoInfo *info = &encoder->input_state->info;
  GstFlowReturn ret;
  FrameData *fdata;

  if (G_UNLIKELY (encoder->x265enc == 0))
    goto not_inited;

  fdata = gst_amlx265enc_queue_frame (encoder, frame, info);
  if (!fdata)
    goto invalid_frame;

  ret = gst_amlx265enc_encode_frame (encoder, frame, TRUE);

  /* input buffer is released later on */
  return ret;

/* ERRORS */
not_inited:
  {
    GST_WARNING_OBJECT (encoder, "Got buffer before set_caps was called");
    return GST_FLOW_NOT_NEGOTIATED;
  }
invalid_frame:
  {
    GST_ERROR_OBJECT (encoder, "Failed to map frame");
    return GST_FLOW_ERROR;
  }
}

static GstFlowReturn
gst_amlx265enc_encode_frame (GstAmlX265Enc * encoder,
    GstVideoCodecFrame * input_frame, gboolean send)
{
  GstVideoCodecFrame *frame = NULL;
  GstBuffer *out_buf = NULL;
  GstMapInfo inbuf_info, outbuf_info;
  int encoder_return;
  GstFlowReturn ret = GST_FLOW_OK;
  gboolean update_latency = FALSE;
  vl_frame_type_t frame_type = FRAME_TYPE_AUTO;
  GstVideoInfo *info;

  if (G_UNLIKELY (encoder->x265enc == 0)) {
    if (input_frame)
      gst_video_codec_frame_unref (input_frame);
    return GST_FLOW_NOT_NEGOTIATED;
  }

  GST_OBJECT_LOCK (encoder);
  if (encoder->reconfig) {
    encoder->reconfig = FALSE;
    // TODO
#if 0
    if (encoder->vtable->gst_amlx265encoder_reconfig (encoder->x265enc,
            &encoder->x265param) < 0)
      GST_WARNING_OBJECT (encoder, "Could not reconfigure");
#endif
    update_latency = TRUE;
  }

  if (input_frame) {
    if (GST_VIDEO_CODEC_FRAME_IS_FORCE_KEYFRAME (input_frame)) {
      GST_INFO_OBJECT (encoder, "Forcing key frame");
      frame_type = FRAME_TYPE_IDR;
    }
  }
  GST_OBJECT_UNLOCK (encoder);

  if (G_UNLIKELY (update_latency))
    gst_amlx265enc_set_latency (encoder);

  encoder_return = -1;
  info = &encoder->input_state->info;
  if (gst_buffer_map (input_frame->input_buffer, &inbuf_info, GST_MAP_READ)) {
    GstVideoFormat vfmt = info->finfo->format;
    int fmt = 0;
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
    out_buf = gst_buffer_new_allocate (NULL, 512*1024, NULL);
    gst_buffer_map (out_buf, &outbuf_info, GST_MAP_WRITE);
    encoder_return = encoder->vtable->vl_video_encoder_encode (encoder->x265enc,
        frame_type, inbuf_info.data, inbuf_info.size, outbuf_info.data, fmt);
    gst_buffer_unmap (out_buf, &outbuf_info);
    gst_buffer_unmap (input_frame->input_buffer, &inbuf_info);
  }

  if (encoder_return < 0) {
    GST_ELEMENT_ERROR (encoder, STREAM, ENCODE, ("Encode x265 frame failed."),
        ("gst_amlx265encoder_encode return code=%d", encoder_return));
    ret = GST_FLOW_ERROR;
    /* Make sure we finish this frame */
    frame = input_frame;
    goto out;
  }

  /* Input frame is now queued */
  if (input_frame)
    gst_video_codec_frame_unref (input_frame);

  gst_buffer_resize (out_buf, 0, encoder_return);
  frame = gst_video_encoder_get_frame (GST_VIDEO_ENCODER (encoder), input_frame->system_frame_number);
  g_assert (frame || !send);

  if (!send || !frame) {
    ret = GST_FLOW_OK;
    goto out;
  }

  frame->output_buffer = out_buf;

  frame->pts = input_frame->pts;
  frame->dts = -1;

  GST_LOG_OBJECT (encoder,
      "output: dts %" G_GINT64_FORMAT " pts %" G_GINT64_FORMAT,
      (gint64) frame->dts, (gint64) frame->pts);

  if (frame_type == FRAME_TYPE_IDR) {
    GST_DEBUG_OBJECT (encoder, "Output keyframe");
    GST_VIDEO_CODEC_FRAME_SET_SYNC_POINT (frame);
  }

out:
  if (frame) {
    gst_amlx265enc_dequeue_frame (encoder, frame);
    ret = gst_video_encoder_finish_frame (GST_VIDEO_ENCODER (encoder), frame);
  }

  return ret;
}

static void
gst_amlx265enc_flush_frames (GstAmlX265Enc * encoder, gboolean send)
{
}

static void
gst_amlx265enc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAmlX265Enc *encoder = GST_AMLX265ENC (object);

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
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (encoder);
}

static void
gst_amlx265enc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAmlX265Enc *encoder = GST_AMLX265ENC (object);
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
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

  GST_OBJECT_UNLOCK (encoder);
  return;
}

static gboolean
amlx265enc_init (GstPlugin * amlx265enc)
{
  GST_DEBUG_CATEGORY_INIT (gst_amlx265enc_debug, "amlx265enc", 0,
      "amlogic h265 encoding element");

  default_vtable.module = NULL;

  if (!load_x265_libraries ())
    return FALSE;

  return gst_element_register (amlx265enc, "amlx265enc", GST_RANK_PRIMARY,
      GST_TYPE_AMLX265ENC);
}

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    amlx265enc,
    "H265 encoder plugins for amlogic",
    amlx265enc_init,
    VERSION,
    "LGPL",
    "amlogic h265 ecoding",
    "http://openlinux.amlogic.com"
)
