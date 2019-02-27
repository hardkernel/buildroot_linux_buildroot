#ifndef _ONVIF_RTSP_CONFIG_
#define _ONVIF_RTSP_CONFIG_

#include <string>
#include <memory>

typedef struct config_network {
  std::string route;
  std::string address;
  std::string port;
} CFG_NETWORK_t;

typedef struct config_auth {
  std::string username;
  std::string password;
} CFG_AUTH_t;

typedef struct config_video {
  std::string device;
  std::string framerate;
  std::pair<std::string, std::string> scale;
  std::string bitrate;
  std::string gop;
  // use x265
  bool use_x265;
} CFG_VIDEO_t;

typedef struct config_backchannel {
  std::string device;
  std::string clock_rate;
  std::string encoding;
} CFG_BACKCHANNEL_t;

typedef struct config_audio {
  std::string device;
  std::string bitrate;
  std::string samplerate;
  std::string codec;
} CFG_AUDIO_t;

typedef struct config_storage {
  std::string location; // storage location (directory)
  std::string chunk_duration; // video chunk duration (minutes)
  long reserved_space_size; // reserved space in MBytes
  bool enabled;
} CFG_STORAGE_t;

typedef struct config_debug {
  // Disable audio
  bool disable_audio;

  // Disable backchannel
  bool disable_backchannel;

  std::string pipeline;
  std::string backchannel;
} CFG_DEBUG_t;

typedef struct config {
  // Server config
  CFG_NETWORK_t network;
  CFG_AUTH_t auth;

  // Video options
  CFG_VIDEO_t video;

  // Audio options
  CFG_AUDIO_t audio;

  // Storage options
  CFG_STORAGE_t storage;

  // Backchannel options
  CFG_BACKCHANNEL_t backchannel;

  // Overlay options
  std::string overlay_options;


  // Face detection
  bool face_detection;

  // Custom gstreamer pipeline
  CFG_DEBUG_t debug;
} CONFIG_t;

void rtsp_config_init(std::shared_ptr<CONFIG_t> &config);

#endif /* _ONVIF_RTSP_CONFIG_ */
