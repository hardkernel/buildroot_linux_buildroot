#ifndef _ONVIF_RTSP_PIPELINE_
#define _ONVIF_RTSP_PIPELINE_

#include <gst/gst.h>
#include "onvif_rtsp_config.h"

typedef struct rtsp_source_pipeline {
  GstElement *pipeline;

  GstElement *vsrc;

  GstElement *vsink;
  GstElement *asink;

  GstPad *vsink_sink_pad;
  GstPad *asink_sink_pad;

  GstCaps *vsink_caps;
  GstCaps *asink_caps;

} PIPELINE_SRC_t;

typedef struct rtsp_storage_pipeline {
  GstElement *pipeline;

  GstElement *vsrc;
  GstElement *asrc;

  GstElement *parser;
  GstElement *muxer;
  GstElement *filesink;

  GstPad *parser_src_pad;
  GstPad *muxer_vsink_pad;
  GstPad *muxer_asink_pad;
  GstPad *filesink_sink_pad;

  GstPadProbeInfo *vbuffer_probe;

  GstElement *adepay;
  GstPad *adepay_src_pad;
  GstPadProbeInfo *abuffer_probe;

  bool is_switching;
  bool is_running;
  bool is_audio_pad_blocked;

} PIPELINE_STO_t;

typedef struct rtsp_pipeline {
  PIPELINE_SRC_t src;
  PIPELINE_STO_t sto;
} RTSP_PIPELINE_t;

std::string pipeline_create_src(std::shared_ptr<RTSP_CONFIG_t> &config);
std::string pipeline_create_sto(std::shared_ptr<RTSP_CONFIG_t> &config);
std::string pipeline_create_rtp(std::shared_ptr<RTSP_CONFIG_t> &config);
std::string pipeline_create_backchannel(std::shared_ptr<RTSP_CONFIG_t> &config);

#endif /* _ONVIF_RTSP_SERVER_ */
