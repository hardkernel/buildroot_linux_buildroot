#include "onvif_rtsp_config.h"
#include <iostream>
#include <string.h>

#define PROPERTY_DUMMY 1

#if PROPERTY_DUMMY
#define IPC_PROPERTY_VALUE_MAXLEN 256
bool ipc_get_property (const char *key, char *value) {
  return false;
}

#endif

static void rtsp_config_parse_property (std::shared_ptr<CONFIG_t> &config) {
  char value[IPC_PROPERTY_VALUE_MAXLEN];

  // Address
  if (ipc_get_property ("rtsp/network/address", value)) {
    config->network.address = value;
  }

  // Port
  if (ipc_get_property ("rtsp/network/port", value)) {
    config->network.port = value;
  }

  // Route
  if (ipc_get_property ("rtsp/network/route", value)) {
    config->network.route = value;
  }

  // Username
  if (ipc_get_property ("rtsp/auth/username", value)) {
    config->auth.username = value;
  }

  // Password
  if (ipc_get_property ("rtsp/auth/password", value)) {
    config->auth.password = value;
  }

  // Device
  if (ipc_get_property ("ipc/video/device", value)) {
    config->video.device = value;
  }

  // Framerate
  if (ipc_get_property ("ipc/video/framerate", value)) {
    config->video.framerate = value;
  }

  // Scale
  if (ipc_get_property ("ipc/video/resolution", value)) {
    size_t pos = 0;
    std::string scale_str (value);

    if ((pos = scale_str.find ("x")) == std::string::npos) {
      std::cerr
          << "No x token found between width and height in the scale argument: "
          << scale_str << std::endl
          << "Using default values";
    } else {
      config->video.scale = std::make_pair<std::string, std::string> (
          scale_str.substr (0, pos), scale_str.substr (pos + 1));
    }
  }

  // Bitrate
  if (ipc_get_property ("ipc/video/bitrate", value)) {
    config->video.bitrate = value;
  }

  // GOP
  if (ipc_get_property ("ipc/video/gop", value)) {
    config->video.gop = value;
  }

#if 0
  // Overlay
  if (ipc_get_property ("ipc/overlay/options", value)) {
    config->overlay_options = value;
  }
#endif

  // Use x265enc
  if ( ipc_get_property ("ipc/video/codec", value)) {
    if (strcmp (value, "x265") == 0) {
      config->video.use_x265 = true;
    } else {
      config->video.use_x265 = false;
    }
  }

  // ISP options
  // WDR
  if ( ipc_get_property ("ipc/isp/wdr/enabled", value)) {
    if (strcmp (value, "true") == 0) {
      config->isp.wdr_enabled = true;
    } else {
      config->isp.wdr_enabled = false;
    }
  }

  // Audio options
  // Audio device
  if (ipc_get_property ("ipc/audio/device", value)) {
    config->audio.device = value;
  }
  // Audio bitrate
  if (ipc_get_property ("ipc/audio/bitrate", value)) {
    config->audio.bitrate = value;
  }
  // Audio samplerate
  if (ipc_get_property ("ipc/audio/samplerate", value)) {
    config->audio.samplerate = value;
  }
  // Audio encoder
  if (ipc_get_property ("ipc/audio/codec", value)) {
    config->audio.codec = value;
  }

  // Backchannel clock rate
  if (ipc_get_property ("ipc/backchannel/clock_rate", value)) {
    config->backchannel.clock_rate = value;
  }

  // Backchannel encoding
  if (ipc_get_property ("ipc/backchannel/encoding", value)) {
    config->backchannel.encoding = value;
  }

  // Backchannel output device
  if (ipc_get_property ("ipc/backchannel/device", value)) {
    config->backchannel.device = value;
  }

  // Storage location
  if (ipc_get_property ("ipc/storage/location", value)) {
    config->storage.location = value;
  }

  // Storage chunk duration
  if (ipc_get_property ("ipc/storage/chunk_duration", value)) {
    config->storage.chunk_duration = value;
  }

  // Storage reserved space size
  if (ipc_get_property ("ipc/storage/reserved_space_size", value)) {
    config->storage.reserved_space_size = atol (value);
  }

  // Storage enabled
  if (ipc_get_property ("ipc/storage/enabled", value)) {
    if (strcmp (value, "false") == 0) {
      config->storage.enabled= false;
    } else {
      config->storage.enabled = true;
    }
  }


  // Face Detection
  if (ipc_get_property ("ipc/face detection/enabled", value)) {
    if (strcmp (value, "false") == 0) {
      config->face_detection = false;
    } else {
      config->face_detection = true;
    }
  }

}

#define DEFAULT_USERNAME ""
#define DEFAULT_PASSWORD ""
#define DEFAULT_ROUTE "/live.sdp"
#define DEFAULT_ADDRESS "0.0.0.0"
#define DEFAULT_PORT "554"

#define DEFAULT_VIDEO_DEVICE "/dev/video0"
#define DEFAULT_FRAMERATE "30"
#define DEFAULT_WIDTH "1920"
#define DEFAULT_HEIGHT "1080"
#define DEFAULT_TIME_ENABLED false
#define DEFAULT_VIDEO_BITRATE "2000"
#define DEFAULT_GOP "30"
#define DEFAULT_USE_X265 true

#define DEFAULT_WDR_ENABLED false

#define DEFAULT_AUDIO_DEVICE "test"
#define DEFAULT_AUDIO_BITRATE "64"
#define DEFAULT_SAMPLERATE "8"
#define DEFAULT_AUDIO_CODEC "mp3"
#define DEFAULT_BACKCHANNEL_CLOCK_RATE "8000"
#define DEFAULT_BACKCHANNEL_ENCODING "PCMU"
#define DEFAULT_BACKCHANNEL_DEVICE ""

#define DEFAULT_OVERLAY_OPTIONS ""
#define DEFAULT_FACE_DETECTION_ENABLED false

#define DEFAULT_STORAGE_LOCATION ""
#define DEFAULT_STORAGE_CHUNK_DURATION "5"
#define DEFAULT_STORAGE_RESERVED_SPACE_SIZE 200
#define DEFAULT_STORAGE_ENABLED false

#define DEFAULT_DEBUG_DISABLE_AUDIO false
#define DEFAULT_DEBUG_DISABLE_BACKCHANNEL false
#define DEFAULT_DEBUG_PIPELINE ""
#define DEFAULT_BACKCHANNEL_PIPELINE ""

static void rtsp_config_parse_env (std::shared_ptr<CONFIG_t> &config) {
  // Address
  config->network.address = DEFAULT_ADDRESS;
  if (const char *address = std::getenv ("RTSP_NETWORK_ADDRESS")) {
    config->network.address = address;
  }

  // Port
  config->network.port = DEFAULT_PORT;
  if (const char *port = std::getenv ("RTSP_NETWORK_PORT")) {
    config->network.port = port;
  }

  // Route
  config->network.route = DEFAULT_ROUTE;
  if (const char *route = std::getenv ("RTSP_NETWORK_ROUTE")) {
    config->network.route = route;
  }

  // Username
  config->auth.username = DEFAULT_USERNAME;
  if (const char *username = std::getenv ("RTSP_AUTH_USERNAME")) {
    config->auth.username = username;
  }

  // Password
  config->auth.password = DEFAULT_PASSWORD;
  if (const char *password = std::getenv ("RTSP_AUTH_PASSWORD")) {
    config->auth.password = password;
  }

  // Device
  config->video.device = DEFAULT_VIDEO_DEVICE;
  if (const char *device = std::getenv ("IPC_VIDEO_DEVICE")) {
    config->video.device = device;
  }

  // Framerate
  config->video.framerate = DEFAULT_FRAMERATE;
  if (const char *framerate = std::getenv ("IPC_VIDEO_FRAMERATE")) {
    config->video.framerate = framerate;
  }

  // Scale
  config->video.scale =
      std::make_pair<std::string, std::string> (DEFAULT_WIDTH, DEFAULT_HEIGHT);
  if (const char *scale = std::getenv ("IPC_VIDEO_RESOLUTION")) {
    size_t pos = 0;
    std::string scale_str (scale);

    if ((pos = scale_str.find ("x")) == std::string::npos) {
      std::cerr
          << "No x token found between width and height in the scale argument: "
          << scale_str << std::endl
          << "Using default values";
    } else {
      config->video.scale = std::make_pair<std::string, std::string> (
          scale_str.substr (0, pos), scale_str.substr (pos + 1));
    }
  }

  // Bitrate
  config->video.bitrate = DEFAULT_VIDEO_BITRATE;
  if (const char *bitrate = std::getenv ("IPC_VIDEO_BITRATE")) {
    config->video.bitrate = bitrate;
  }

  // GOP
  config->video.gop = DEFAULT_GOP;
  if (const char *gop = std::getenv ("IPC_VIDEO_GOP")) {
    config->video.gop = gop;
  }

  // Use x265enc
  config->video.use_x265 = DEFAULT_USE_X265;
  if ( const char *use_x265 = std::getenv ("IPC_VIDEO_USE_X265ENC")) {
    if (strcmp (use_x265, "true") == 0) {
      config->video.use_x265 = true;
    } else {
      config->video.use_x265 = false;
    }
  }

  // ISP options
  // WDR
  config->isp.wdr_enabled = DEFAULT_WDR_ENABLED;
  if ( const char *value = std::getenv ("IPC_ISP_WDR_ENABLED")) {
    if (strcmp (value, "true") == 0) {
      config->isp.wdr_enabled = true;
    } else {
      config->isp.wdr_enabled = false;
    }
  }

  // Audio options
  // Audio device
  config->audio.device = DEFAULT_AUDIO_DEVICE;
  if (const char *device = std::getenv ("IPC_AUDIO_DEVICE")) {
    config->audio.device = device;
  }
  // Audio bitrate
  config->audio.bitrate = DEFAULT_AUDIO_BITRATE;
  if (const char *bitrate = std::getenv ("IPC_AUDIO_BITRATE")) {
    config->audio.bitrate = bitrate;
  }
  // Audio samplerate
  config->audio.samplerate = DEFAULT_SAMPLERATE;
  if (const char *samplerate = std::getenv ("IPC_AUDIO_SAMPLERATE")) {
    config->audio.samplerate = samplerate;
  }
  // Audio encoder
  config->audio.codec = DEFAULT_AUDIO_CODEC;
  if (const char *codec = std::getenv ("IPC_AUDIO_CODEC")) {
    config->audio.codec = codec;
  }

  // Backchannel clock rate
  config->backchannel.clock_rate = DEFAULT_BACKCHANNEL_CLOCK_RATE;
  if (const char *value = std::getenv ("IPC_BACKCHANNEL_CLOCK_RATE")) {
    config->backchannel.clock_rate = value;
  }

  // Backchannel encoding
  config->backchannel.encoding = DEFAULT_BACKCHANNEL_ENCODING;
  if (const char *value = std::getenv ("IPC_BACKCHANNEL_ENCODING")) {
    config->backchannel.encoding = value;
  }

  // Backchannel output device
  config->backchannel.device = DEFAULT_BACKCHANNEL_DEVICE;
  if (const char *value = std::getenv ("IPC_BACKCHANNEL_DEVICE")) {
    config->backchannel.device = value;
  }

  // Storage location
  config->storage.location = DEFAULT_STORAGE_LOCATION;
  if (const char *value = std::getenv ("IPC_STORAGE_LOCATION")) {
    config->storage.location = value;
  }

  // Storage chunk duration
  config->storage.chunk_duration = DEFAULT_STORAGE_CHUNK_DURATION;
  if (const char *value = std::getenv ("IPC_STORAGE_CHUNK_DURATION")) {
    config->storage.chunk_duration = value;
  }

  // Storage reserved space size
  config->storage.reserved_space_size = DEFAULT_STORAGE_RESERVED_SPACE_SIZE;
  if (const char *value = std::getenv ("IPC_STORAGE_RESERVED_SPACE_SIZE")) {
    config->storage.reserved_space_size = atol (value);
  }

  // Storage enabled
  config->storage.enabled = DEFAULT_STORAGE_ENABLED;
  if (const char *value = std::getenv ("IPC_STORAGE_ENABLED")) {
    if (strcmp (value, "false") == 0) {
      config->storage.enabled= false;
    } else {
      config->storage.enabled = true;
    }
  }


  // Overlay
  config->overlay_options = DEFAULT_OVERLAY_OPTIONS;
  if (const char *option = std::getenv ("IPC_OVERLAY_OPTIONS")) {
    config->overlay_options = option;
  }

  // Face Detection
  config->face_detection = DEFAULT_FACE_DETECTION_ENABLED;
  if ( const char *face_detection = std::getenv ("IPC_FACE_DETECTION_ENABLED")) {
    if (strcmp (face_detection, "false") == 0) {
      config->face_detection = false;
    } else {
      config->face_detection = true;
    }
  }

  // --- debug options ---
  // Disable audio
  config->debug.disable_audio = DEFAULT_DEBUG_DISABLE_AUDIO;
  if (const char *value = std::getenv ("IPC_DEBUG_DISABLE_AUDIO")) {
    if (strcmp (value, "false") == 0) {
      config->debug.disable_audio = false;
    } else {
      config->debug.disable_audio = true;
    }
  }
  // Disable backchannel
  config->debug.disable_backchannel = DEFAULT_DEBUG_DISABLE_BACKCHANNEL;
  if (const char *value = std::getenv ("IPC_DEBUG_DISABLE_BACKCHANNEL")) {
    if (strcmp (value, "false") == 0) {
      config->debug.disable_backchannel = false;
    } else {
      config->debug.disable_backchannel = true;
    }
  }
  // Customize pipeline
  config->debug.pipeline = DEFAULT_DEBUG_PIPELINE;
  if (const char *pipeline = std::getenv ("IPC_DEBUG_PIPELINE")) {
    config->debug.pipeline = pipeline;
  }

  config->debug.backchannel = DEFAULT_BACKCHANNEL_PIPELINE;
  if (const char *pipeline = std::getenv ("IPC_DEBUG_BACKCHANNEL_PIPELINE")) {
    config->debug.backchannel = pipeline;
  }
}

void rtsp_config_init (std::shared_ptr<CONFIG_t> &config) {
  // get configuration from property
  rtsp_config_parse_property (config);

  // get configuration from ENV, overwrite the previous
  rtsp_config_parse_env (config);
}

