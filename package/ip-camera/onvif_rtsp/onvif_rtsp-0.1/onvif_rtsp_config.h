#ifndef _ONVIF_RTSP_CONFIG_
#define _ONVIF_RTSP_CONFIG_

#include <string>
#include <memory>

typedef struct rtsp_config_network {
  std::string route;
  std::string address;
  std::string port;
} CFG_NETWORK_t;

typedef struct rtsp_config_auth {
  std::string username;
  std::string password;
} CFG_AUTH_t;

typedef struct rtsp_config_video {
  std::string device;
  std::string framerate;
  std::pair<std::string, std::string> scale;
  std::string bitrate;
  std::string gop;
  // use x265
  bool use_x265;
} CFG_VIDEO_t;

typedef struct rtsp_config_backchannel {
  std::string clock_rate;
  std::string encoding;
} CFG_BACKCHANNEL_t;

typedef struct rtsp_config_audio {
  std::string device;
  std::string bitrate;
  std::string samplerate;
  std::string codec;
  CFG_BACKCHANNEL_t backchannel;
} CFG_AUDIO_t;

typedef struct rtsp_config_debug_pipeline {
  std::string pipeline;
  std::string backchannel;
} CFG_DEBUG_t;

typedef struct rtsp_config {
  // Server config
  CFG_NETWORK_t network;
  CFG_AUTH_t auth;

  // Video options
  CFG_VIDEO_t video;

  // Audio options
  CFG_AUDIO_t audio;

  // Overlay options
  std::string overlay_options;

  // Face detection
  bool face_detection;

  // Custom gstreamer pipeline
  CFG_DEBUG_t debug;
} RTSP_CONFIG_t;

void rtsp_config_init(std::shared_ptr<RTSP_CONFIG_t> &config);

#endif /* _ONVIF_RTSP_CONFIG_ */
