#include "onvif_rtsp_pipeline.h"
#include <string.h>

std::string pipeline_create_src (std::shared_ptr<CONFIG_t> &config) {
  std::string pipeline_desc = "";

  // video
  pipeline_desc += "v4l2src name=vsrc device=";
  pipeline_desc += config->video.device;

  pipeline_desc += " ! videoscale ! video/x-raw,width=";
  pipeline_desc += config->video.scale.first;
  pipeline_desc += ",height=";
  pipeline_desc += config->video.scale.second;
  pipeline_desc += ",framerate=";
  pipeline_desc += config->video.framerate;
  pipeline_desc += "/1";

  if (config->face_detection) {
    pipeline_desc += " ! amlyoloface";
  }

  pipeline_desc += " ! amloverlay";
  if (not config->overlay_options.empty ()) {
    pipeline_desc += " ";
    pipeline_desc += config->overlay_options;
  }

  pipeline_desc += " ! amlvenc framerate=";
  pipeline_desc += config->video.framerate;

  pipeline_desc += " bitrate=";
  pipeline_desc += config->video.bitrate;

  pipeline_desc += " gop=";
  pipeline_desc += config->video.gop;

  if (config->video.use_x265) {
    pipeline_desc += " ! rtph265pay";
  } else {
    pipeline_desc += " ! rtph264pay";
  }
  pipeline_desc += " config-interval=1 pt=96";
  pipeline_desc += " ! udpsink name=vsink host=224.1.1.1 auto-multicast=true port=2222 sync=false";
  pipeline_desc += " multicast-iface=lo";

  // audio
  if (!config->debug.disable_audio) {
    if (config->audio.device == "test") {
      pipeline_desc += " audiotestsrc is-live=true wave=white-noise";
    } else {
      pipeline_desc += " alsasrc device=";
      pipeline_desc += config->audio.device;
    }

    if (config->audio.codec == "mulaw") {
      pipeline_desc += " ! mulawenc";
    } else if (config->audio.codec == "opus") {
      pipeline_desc += " ! opusenc";
    } else if (config->audio.codec == "mp3") {
      pipeline_desc += " ! lamemp3enc";
    }

    if (config->audio.codec == "mulaw") {
      pipeline_desc += " ! rtppcmupay";
    } else if (config->audio.codec == "opus") {
      pipeline_desc += " ! rtpopuspay";
    } else if (config->audio.codec == "mp3") {
      pipeline_desc += " ! rtpmpapay";
    }

    pipeline_desc += " pt=96";
    pipeline_desc += " ! udpsink name=asink host=224.1.1.1 auto-multicast=true port=2223 sync=false";
    pipeline_desc += " multicast-iface=lo";
  }

  g_print ("Source pipeline:\n  %s\n", pipeline_desc.c_str ());
  return pipeline_desc;
}

std::string pipeline_create_sto (std::shared_ptr<CONFIG_t> &config) {
  std::string pipeline_desc = "";

  // video
  pipeline_desc += "qtmux name=muxer";
  pipeline_desc += " udpsrc name=vsrc multicast-group=224.1.1.1 auto-multicast=true port=2222";
  pipeline_desc += " multicast-iface=lo";

  if (config->video.use_x265) {
    pipeline_desc += " ! rtph265depay ! h265parse";
  } else {
    pipeline_desc += " ! rtph264depay ! h264parse";
  }
  pipeline_desc += " name=parser ! muxer.";

  // audio
  if (!config->debug.disable_audio) {
    pipeline_desc += " udpsrc name=asrc multicast-group=224.1.1.1 auto-multicast=true port=2223";
    pipeline_desc += " multicast-iface=lo";

    if (config->audio.codec == "mulaw") {
      pipeline_desc += " ! rtppcmudepay";
    } else if (config->audio.codec == "opus") {
      pipeline_desc += " ! rtpopusdepay";
    } else if (config->audio.codec == "mp3") {
      pipeline_desc += " ! rtpmpadepay";
    }
    if (config->audio.codec == "mp3") {
      pipeline_desc += " name=adepay ! mpegaudioparse ! muxer.";
    } else {
      pipeline_desc += " name=adepay ! muxer.";
    }
  }

  pipeline_desc += " muxer. ! filesink name=recordfile async=false";

  g_print ("Storage pipeline:\n  %s\n", pipeline_desc.c_str ());
  return pipeline_desc;
}


std::string pipeline_create_rtp (std::shared_ptr<CONFIG_t> &config) {
  std::string pipeline_desc = " ( ";

  // video
  pipeline_desc += "udpsrc name=vsrc multicast-group=224.1.1.1 auto-multicast=true port=2222";
  pipeline_desc += " multicast-iface=lo";

  if (config->video.use_x265) {
    pipeline_desc += " ! rtph265depay ! rtph265pay";
  } else {
    pipeline_desc += " ! rtph264depay ! rtph264pay";
  }
  pipeline_desc += " pt=96 config-interval=1 name=pay0";

  // audio
  if (!config->debug.disable_audio) {
    pipeline_desc += " udpsrc name=asrc multicast-group=224.1.1.1 auto-multicast=true port=2223";
    pipeline_desc += " multicast-iface=lo";

    if (config->audio.codec == "mulaw") {
      pipeline_desc += " ! rtppcmudepay ! rtppcmupay";
    } else if (config->audio.codec == "opus") {
      pipeline_desc += " ! rtpopusdepay ! rtpopuspay";
    } else if (config->audio.codec == "mp3") {
      pipeline_desc += " ! rtpmpadepay ! mpegaudioparse ! rtpmpapay";
    }
    pipeline_desc += " pt=96 name=pay1";
  }

  pipeline_desc += " )";

  g_print ("Stream pipeline:\n  %s\n", pipeline_desc.c_str ());

  return pipeline_desc;
}

std::string pipeline_create_backchannel (std::shared_ptr<CONFIG_t> &config) {
  if (not config->debug.backchannel.empty ()) {
    return config->debug.backchannel;
  }
  std::string pipeline_desc = " ( ";
  pipeline_desc += "capsfilter caps=\"application/x-rtp,media=audio,payload=0,clock-rate=";
  pipeline_desc += config->backchannel.clock_rate;
  pipeline_desc += ",encoding-name=";
  pipeline_desc += config->backchannel.encoding;
  pipeline_desc += "\" name=depay_backchannel !";
  if (config->backchannel.encoding == "PCMU") {
    pipeline_desc += " rtppcmudepay ! mulawdec";
  }
  if (config->backchannel.device.empty()) {
    pipeline_desc += " ! fakesink async=false";
  } else {
    pipeline_desc += " ! audioresample ! alsasink device=";
    pipeline_desc += config->backchannel.device;
    pipeline_desc += " async=false sync=false";
  }
  pipeline_desc += " )";

  g_print ("Backchannel pipeline:\n  %s\n", pipeline_desc.c_str ());

  return pipeline_desc;
}

