// Copyright 2017 The Cobalt Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "starboard/shared/starboard/media/media_support_internal.h"

#include "starboard/configuration.h"
#include "starboard/media.h"
#include "starboard/string.h"
#include "starboard/log.h"
#include <stdlib.h>
#include <string>
#include <map>

namespace {
struct SupportedVideoCaps {
  struct VideoCaps {
    int64_t bitrate;
    int width;
    int height;
    int fps;
  };
  std::map<std::string, VideoCaps> all_caps;

  VideoCaps *GetVideoCap(SbMediaVideoCodec video_codec, bool decode_to_texture) {
    std::string name = "none";
    if (decode_to_texture) name = "vr";
    else if (video_codec == kSbMediaVideoCodecVp9) name = "vp9";
    else if (video_codec == kSbMediaVideoCodecH264) name = "h264";
    auto it = all_caps.find(name);
    return it != all_caps.end() ? (&it->second) : NULL;
  }

  void InitVideoCap(void) {
      const char *env = getenv("COBALT_SUPPORTED_VIDEO_CAPS");
      if (!env) env = "vp9:3840x2160@60<0,h264:3840x2160@30<0,vr:1920x1080@30<0,hdr:3840x2160@30<0";
      SB_LOG(INFO) << "InitVideoCap from string \"" << env << "\"";
      while (true) {
          char codec[32];
          int64_t bitrate;
          int width, height, fps, len = 0;
          if (sscanf(env, "%31[^:]:%dx%d@%d<%lld%n", codec, &width, &height, &fps, &bitrate, &len) != 5) {
              SB_LOG(INFO) << "failed to get video caps \"" << env << "\"";
              break;
          }
          bitrate = bitrate ? bitrate : SB_MEDIA_MAX_VIDEO_BITRATE_IN_BITS_PER_SECOND;
          all_caps[codec] = {bitrate, width, height, fps};
          SB_LOG(INFO) << "add video cap " << codec << ":" << width << "x" << height << "@" << fps << "<" << bitrate;
          env += len;
          if (*env++ == '\0') break;
      }
  }
};
} // namespace

SB_EXPORT bool SbMediaIsVideoSupported(SbMediaVideoCodec video_codec,
                                       int frame_width,
                                       int frame_height,
                                       int64_t bitrate,
                                       int fps,
                                       bool decode_to_texture_required) {
  static SupportedVideoCaps * supported_caps = nullptr;
  if (supported_caps == nullptr) {
    supported_caps = new SupportedVideoCaps;
    supported_caps->InitVideoCap();
  }
  SupportedVideoCaps::VideoCaps *caps = supported_caps->GetVideoCap(video_codec, decode_to_texture_required);
  return (caps != NULL) && frame_width <= caps->width &&
         frame_height <= caps->height && bitrate <= caps->bitrate &&
         fps <= caps->fps;
}
