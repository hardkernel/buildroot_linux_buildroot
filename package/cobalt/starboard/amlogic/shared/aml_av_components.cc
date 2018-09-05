#include "aml_av_components.h"
#include "starboard/shared/starboard/player/filter/audio_renderer_sink_impl.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

namespace starboard {
namespace shared {
namespace starboard {
namespace player {
namespace filter {

AmlAVCodec::AmlAVCodec()
    : eos_state(0), name("none: "), prerolled(false), feed_data_func(nullptr) {
  codec_param = (codec_para_t *)calloc(1, sizeof(codec_para_t));
}

AmlAVCodec::~AmlAVCodec() {
  if (codec_param) {
    codec_reset(codec_param);
    codec_close(codec_param);
    free(codec_param);
    codec_param = NULL;
  }
}

bool AmlAVCodec::AVInitCodec() {
  func_check_eos = std::bind(&AmlAudioRenderer::AVCheckDecoderEos, this);
  if (codec_param->has_audio) {
    amthreadpool_system_init();
  }
  if (codec_param->stream_type == STREAM_TYPE_ES_VIDEO) {
    name = "video: ";
    isvideo = true;
    CLOG(WARNING) << "init video cocec " << codec_param->video_type;
  } else if (codec_param->stream_type == STREAM_TYPE_ES_AUDIO) {
    name = "audio: ";
    isvideo = false;
    CLOG(WARNING) << "init video cocec " << codec_param->audio_type;
    amsysfs_set_sysfs_int("/sys/class/tsync/enable", 1);
    amsysfs_set_sysfs_int("/sys/class/tsync/mode", 1);
    amsysfs_set_sysfs_int("/sys/class/tsync/slowsync_enable", 0);
    amsysfs_set_sysfs_int("/sys/class/tsync/av_threshold_min", 720000);
  }
  codec_param->noblock = 1;
  int ret = codec_init(codec_param);
  if (ret != CODEC_ERROR_NONE) {
    CLOG(ERROR) << "failed to intialize codec " << ret;
    free(codec_param);
    codec_param = NULL;
  }
  codec_set_syncenable(codec_param, 1);
}

void AmlAVCodec::AVInitialize(const ErrorCB &error_cb,
                              const PrerolledCB &prerolled_cb,
                              const EndedCB &ended_cb) {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return;
  }
  error_cb_ = error_cb;
  prerolled_cb_ = prerolled_cb;
  ended_cb_ = ended_cb;
}

void AmlAVCodec::AVPlay() {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return;
  }
  int ret = codec_resume(codec_param);
  if (ret != CODEC_ERROR_NONE) {
    CLOG(ERROR) << "failed to resume codec " << ret;
  }
  CLOG(INFO) << "codec_resumed";
}

void AmlAVCodec::AVPause() {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return;
  }
  int ret = codec_pause(codec_param);
  if (ret != CODEC_ERROR_NONE) {
    CLOG(ERROR) << "failed to pause codec " << ret;
  }
  CLOG(INFO) << "codec_paused";
}

void AmlAVCodec::AVSetPlaybackRate(double playback_rate) {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return;
  }
  if (isvideo) {
    if ((playback_rate > 0.99) && (playback_rate < 1.01)) {
      // set_tsync_enable(1);
      //codec_set_video_playrate(codec_param, (int)(1 * (1 << 16)));
    } else {
      //codec_set_video_playrate(codec_param, (int)(playback_rate * (1 << 16)));
    }
  }
}

void AmlAVCodec::AVSeek(SbTime seek_to_time) {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return;
  }
  int ret = codec_reset(codec_param);
  if (ret != CODEC_ERROR_NONE) {
    CLOG(ERROR) << "failed to reset codec " << ret;
  }
  if (!isvideo) {
    unsigned long pts90k = seek_to_time * 90 / kSbTimeMillisecond;
    char buf[64];
    sprintf(buf, "0x%lx", pts90k+1);
    amsysfs_set_sysfs_str("/sys/class/tsync/pts_audio", buf);
  }
  prerolled = false;
  CLOG(WARNING) << "seek to " << seek_to_time/1000000.0;
}

SbTime AmlAVCodec::AVGetCurrentMediaTime(bool *is_playing,
                                         bool *is_eos_played) {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return kSbTimeMax;
  }
  int pts;
  if (1 || isvideo)
    pts = codec_get_vpts(codec_param);
  else
    pts = codec_get_apts(codec_param);
  SbTime retpts = kSbTimeMax;
  if (pts == -1) {
    CLOG(ERROR) << "failed to get pts";
  } else {
      retpts = (uint32_t)pts * kSbTimeMillisecond / 90;
  }
  return retpts;
}

bool AmlAVCodec::WriteCodec(uint8_t *data, int size, bool *written) {
  int remain = size;
  while (remain > 0) {
    int writelen = codec_write(codec_param, data, remain);
    if (writelen > 0) {
      data += writelen;
      remain -= writelen;
    } else if (errno == EAGAIN || errno == EINTR) {
      if (remain == size) {
        *written = false;
        return true;
      } else {
          CLOG(ERROR) << "data partially written, need to wait! remain len:" << remain << " size:" << size;
          usleep(1000*10);
      }
      continue;
    } else {
      CLOG(ERROR) << "write codec error ret " << writelen << " errno " << errno;
      return false;
    }
  }
  *written = true;
  return true;
}

bool AmlAVCodec::AVWriteSample(const scoped_refptr<InputBuffer> &input_buffer,
                               bool *written) {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return false;
  }
  struct buf_status bufstat;
  int ret;
  if (isvideo)
    ret = codec_get_vbuf_state(codec_param, &bufstat);
  else
    ret = codec_get_abuf_state(codec_param, &bufstat);
  if (ret != 0) {
    CLOG(ERROR) << "failed to get buffer state ret:" << ret << " errno:" << errno << ":" << strerror(errno);
    *written = false;
    return true;
  }
  uint8_t *data = const_cast<uint8_t *>(input_buffer->data());
  int size = input_buffer->size();
  if (bufstat.free_len < size + 1024 * 32) {
    *written = false;
    return true;
  }
  SbTime pts_sb = input_buffer->timestamp();
  unsigned long pts90k = pts_sb * 90 / kSbTimeMillisecond;
  ret = codec_checkin_pts(codec_param, pts90k+1);
  if (ret != 0) {
    CLOG(ERROR) << "failed to checkin pts ret:" << ret << " errno:" << errno << ":" << strerror(errno);
    *written = false;
    return true;
  }
  bool success = false;
  if (feed_data_func) {
    success = feed_data_func(data, size, written);
  } else {
    success = WriteCodec(data, size, written);
  }
  if (success && *written) {
    if (!prerolled) {
      prerolled = true;
      CLOG(ERROR) << "prerolled ";
      Schedule(prerolled_cb_);
    }
    frame_data.resize(0);
  }
  return success;
}

void AmlAVCodec::AVCheckDecoderEos() {
  if (eos_state == 1) {
    struct buf_status bufstat;
    int ret;
    if (isvideo)
      ret = codec_get_vbuf_state(codec_param, &bufstat);
    else
      ret = codec_get_abuf_state(codec_param, &bufstat);
    if (ret != 0) {
      CLOG(ERROR) << "failed to get buffer state " << ret;
      return;
    }
    if (bufstat.read_pointer != last_read_point) {
      last_read_point = bufstat.read_pointer;
      rp_freeze_time = SbTimeGetMonotonicNow() + kSbTimeMillisecond * 100;
    } else if (SbTimeGetMonotonicNow() > rp_freeze_time) {
      SbLogFormatF("%sbuffer freeze, size:%x datalen:%x free:%x rp:%x wp:%x\n",
                   name.c_str(), bufstat.size, bufstat.data_len, bufstat.free_len,
                   bufstat.read_pointer, bufstat.write_pointer);
      ended_cb_();
      eos_state = 2;
      return;
    }
    Schedule(func_check_eos, kSbTimeMillisecond * 30);
  }
}

void AmlAVCodec::AVWriteEndOfStream() {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return;
  }
  CLOG(WARNING) << "WriteEndOfStream eos state " << eos_state;
  if (eos_state == 0) {
    eos_state = 1;
    last_read_point = 0;
    AVCheckDecoderEos();
  }
}

void AmlAVCodec::AVSetVolume(double volume) {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return;
  }
  CLOG(WARNING) << "AVSetVolume " << volume;
}

bool AmlAVCodec::AVIsEndOfStreamWritten() const {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return false;
  }
  return bool(eos_state);
}

int AmlAVCodec::AVGetDroppedFrames() const {
  if (!codec_param) {
    CLOG(ERROR) << "codec does not initialize";
    return 0;
  }
  return 0;
}

AmlAudioRenderer::AmlAudioRenderer(SbMediaAudioCodec audio_codec,
                                   SbMediaAudioHeader audio_header,
                                   SbDrmSystem drm_system) {
  codec_param->audio_type = AFORMAT_AAC;
  codec_param->stream_type = STREAM_TYPE_ES_AUDIO;
  codec_param->has_audio = 1;
  codec_param->has_video = 1;
  AVInitCodec();
}

AmlVideoRenderer::AmlVideoRenderer(SbMediaVideoCodec video_codec,
                                   SbDrmSystem drm_system,
                                   SbPlayerOutputMode output_mode,
                                   SbDecodeTargetGraphicsContextProvider
                                       *decode_target_graphics_context_provider) {
  codec_param->has_video = 1;
  if (video_codec == kSbMediaVideoCodecVp9) {
    codec_param->video_type = VFORMAT_VP9;
    codec_param->am_sysinfo.format = VIDEO_DEC_FORMAT_VP9;
    feed_data_func = std::bind(&AmlVideoRenderer::WriteVP9Sample, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  } else if (video_codec == kSbMediaVideoCodecH264) {
    codec_param->video_type = VFORMAT_H264;
    codec_param->am_sysinfo.format = VIDEO_DEC_FORMAT_H264;
  }
  codec_param->stream_type = STREAM_TYPE_ES_VIDEO;
  AVInitCodec();
  bound_x = bound_y = bound_w = bound_h = 0;
}

// Both of the following two functions can be called on any threads.
void AmlVideoRenderer::SetBounds(int z_index, int x, int y, int width,
                                 int height) {
  if (!codec_param) {
    CLOG(ERROR) << "video codec does not initialize";
    return;
  }
  if ((bound_x != x) || (bound_y != y) || (bound_w != width) ||
      (bound_h != height)) {
    codec_utils_set_video_position(x, y, width, height, 0);
    CLOG(INFO) << "SetBounds z:" << z_index << " x:" << x << " y:" << y
               << " w:" << width << " h:" << height;
    bound_x = x;
    bound_y = y;
    bound_w = width;
    bound_h = height;
  }
}

SbDecodeTarget AmlVideoRenderer::GetCurrentDecodeTarget() {
  if (!codec_param) {
    CLOG(ERROR) << "video codec does not initialize";
    return kSbDecodeTargetInvalid;
  }
  return kSbDecodeTargetInvalid;
}

bool AmlVideoRenderer::WriteVP9Sample(uint8_t *buf, int dsize, bool *written) {
  unsigned char marker;
  int frame_number;
  int cur_frame, cur_mag, mag, index_sz, offset[9], size[8], tframesize[9];
  int mag_ptr;
  int ret;
  unsigned char *old_header = NULL;
  int total_datasize = 0;

  marker = buf[dsize - 1];
  if ((marker & 0xe0) == 0xc0) {
    frame_number = (marker & 0x7) + 1;
    mag = ((marker >> 3) & 0x3) + 1;
    index_sz = 2 + mag * frame_number;
    offset[0] = 0;
    mag_ptr = dsize - mag * frame_number - 2;
    if (buf[mag_ptr] != marker) {
      CLOG(ERROR) << " Wrong marker2 : " << marker << " != " << buf[mag_ptr];
      return false;
    }
    mag_ptr++;
    for (cur_frame = 0; cur_frame < frame_number; cur_frame++) {
      size[cur_frame] = 0; // or size[0] = bytes_in_buffer - 1; both OK
      for (cur_mag = 0; cur_mag < mag; cur_mag++) {
        size[cur_frame] = size[cur_frame] | (buf[mag_ptr] << (cur_mag * 8));
        mag_ptr++;
      }
      offset[cur_frame + 1] = offset[cur_frame] + size[cur_frame];
      if (cur_frame == 0)
        tframesize[cur_frame] = size[cur_frame];
      else
        tframesize[cur_frame] = tframesize[cur_frame - 1] + size[cur_frame];
      total_datasize += size[cur_frame];
    }
  } else {
    frame_number = 1;
    offset[0] = 0;
    size[0] = dsize; // or size[0] = bytes_in_buffer - 1; both OK
    total_datasize += dsize;
    tframesize[0] = dsize;
  }
  if (total_datasize > dsize) {
        CLOG(ERROR) << "DATA overflow : " << total_datasize << " > " << dsize;
        return false;
  }
  frame_data.resize(0);
  if (frame_number >= 1) {
    for (cur_frame = 0; cur_frame < frame_number; cur_frame++) {
      int framesize = size[cur_frame];
      int oldframeoff = tframesize[cur_frame] - framesize;
      uint8_t fdata[16];
      uint8_t *old_framedata = buf + oldframeoff;

      framesize += 4;
      /*add amlogic frame headers.*/
      fdata[0] = (framesize >> 24) & 0xff;
      fdata[1] = (framesize >> 16) & 0xff;
      fdata[2] = (framesize >> 8) & 0xff;
      fdata[3] = (framesize >> 0) & 0xff;
      fdata[4] = ((framesize >> 24) & 0xff) ^ 0xff;
      fdata[5] = ((framesize >> 16) & 0xff) ^ 0xff;
      fdata[6] = ((framesize >> 8) & 0xff) ^ 0xff;
      fdata[7] = ((framesize >> 0) & 0xff) ^ 0xff;
      fdata[8] = 0;
      fdata[9] = 0;
      fdata[10] = 0;
      fdata[11] = 1;
      fdata[12] = 'A';
      fdata[13] = 'M';
      fdata[14] = 'L';
      fdata[15] = 'V';
      frame_data.insert(frame_data.end(), fdata, fdata+16);
      framesize -= 4;
      frame_data.insert(frame_data.end(), old_framedata, old_framedata+framesize);
    }
    return WriteCodec(&frame_data[0], frame_data.size(), written);
  }
  CLOG(ERROR) << "Wrong frame_number:" << frame_number << " size:" << dsize;
  return false;
}

} // namespace filter
} // namespace player
} // namespace starboard
} // namespace shared
} // namespace starboard
