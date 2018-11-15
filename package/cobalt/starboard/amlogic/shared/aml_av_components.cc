#include "aml_av_components.h"
#include "starboard/shared/starboard/player/filter/audio_renderer_sink_impl.h"
#include "starboard/shared/starboard/drm/drm_system_internal.h"
#if defined(COBALT_WIDEVINE_OPTEE)
#include "widevine/drm_system_widevine.h"
#include "third_party/starboard/amlogic/shared/ce_cdm/wv14/secmem_tz.h"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

namespace starboard {
namespace shared {
namespace starboard {
namespace player {
namespace filter {

#if defined(COBALT_WIDEVINE_OPTEE)
uint8_t * AmlAVCodec::sec_drm_mem = NULL;
uint8_t * AmlAVCodec::sec_drm_mem_virt = NULL;
int AmlAVCodec::sec_drm_mem_off = 0;
int AmlAVCodec::sec_mem_size = 1024*1024*2;
int AmlAVCodec::sec_mem_pos = 0;
#endif

AmlAVCodec::AmlAVCodec()
    : eos_state(0), name("none: "), prerolled(false), feed_data_func(nullptr) {
  codec_param = (codec_para_t *)calloc(1, sizeof(codec_para_t));
  dump_fp = NULL;
}

AmlAVCodec::~AmlAVCodec() {
  if (codec_param) {
    codec_reset(codec_param);
    codec_close(codec_param);
#if defined(COBALT_WIDEVINE_OPTEE)
    if (codec_param->drmmode == 1) {
      if (sec_drm_mem) {
        Secure_ReleaseResource();
        sec_drm_mem = NULL;
      }
      if (sec_drm_mem_virt) {
          munmap(sec_drm_mem_virt, sec_mem_size);
          sec_drm_mem_virt = NULL;
      }
      sec_mem_pos = 0;
    }
#endif
    free(codec_param);
    codec_param = NULL;
    if (dump_fp) {
      fclose(dump_fp);
      dump_fp = NULL;
    }
  }
}

bool AmlAVCodec::AVInitCodec() {
  func_check_eos = std::bind(&AmlAudioRenderer::AVCheckDecoderEos, this);
  if (codec_param->has_audio) {
    amthreadpool_system_init();
  }
  const char * dumpfile = NULL;
  if (codec_param->stream_type == STREAM_TYPE_ES_VIDEO) {
    name = "video: ";
    isvideo = true;
    dumpfile = getenv("COBALT_DUMP_VIDEO");
    CLOG(WARNING) << "init video cocec " << codec_param->video_type;
  } else if (codec_param->stream_type == STREAM_TYPE_ES_AUDIO) {
    name = "audio: ";
    isvideo = false;
    dumpfile = getenv("COBALT_DUMP_AUDIO");
    CLOG(WARNING) << "init audio cocec " << codec_param->audio_type;
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
  if (dumpfile) {
    dump_fp = fopen(dumpfile, "wb");
    CLOG(WARNING) << "dump ES file to " << dumpfile << " fp: " << (void*)dump_fp;
  }
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
  if (isvideo)
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
      if (0 && remain == size) {
        *written = false;
        return true;
      } else {
          // TODO: need to find out a way to exit playback immediately
          CLOG(ERROR) << "data partially written, need to wait! remain len:" << remain << " size:" << size;
          usleep(1000*10);
      }
      continue;
    } else {
      CLOG(ERROR) << "write codec error ret " << writelen << " errno " << errno;
      return false;
    }
  }
  if (dump_fp) {
#if defined(COBALT_WIDEVINE_OPTEE)
    if (codec_param->drmmode) {
      drminfo_t *drminfo = (drminfo_t *)data;
      if (drminfo->drm_pktsize + sizeof(drminfo_t) == size) {
        CLOG(WARNING) << "dump clear frame pts:" << pts_sb / 1000000.0
                      << " size:" << drminfo->drm_pktsize << " at "
                      << ftell(dump_fp);
        fwrite(&drminfo[1], 1, drminfo->drm_pktsize, dump_fp);
      } else if (sec_drm_mem_virt) {
        uint8_t *secdata = (uint8_t *)(sec_drm_mem_virt - sec_drm_mem) +
                           (sec_drm_mem_off + drminfo->drm_phy);
        CLOG(WARNING) << "dump drm frame pts:" << pts_sb / 1000000.0
                      << " size:" << drminfo->drm_pktsize
                      << " phy:" << (void *)drminfo->drm_phy
                      << " vir:" << (void *)secdata << " at " << ftell(dump_fp);
        fwrite(secdata, 1, drminfo->drm_pktsize, dump_fp);
      } else {
        CLOG(WARNING) << "can't dump drm frame pts:" << pts_sb / 1000000.0
                      << " drm_pktsize:" << drminfo->drm_pktsize
                      << " phy:" << (void *)drminfo->drm_phy
                      << " size:" << size;
      }
    } else
#endif
    {
      CLOG(WARNING) << "dump frame pts:" << pts_sb / 1000000.0
                    << " size:" << size << " ptr:" << (void *)data << " at "
                    << ftell(dump_fp);
      fwrite(data, 1, size, dump_fp);
    }
  }
  *written = true;
  return true;
}

static bool AddVP9Header(uint8_t *buf, int dsize, std::vector<uint8_t> & data)
{
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
      SB_LOG(ERROR) << "AddVP9Header Wrong marker2 : " << marker << " != " << buf[mag_ptr];
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
        SB_LOG(ERROR) << "AddVP9Header DATA overflow : " << total_datasize << " > " << dsize;
        return false;
  }
  data.resize(0);
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
      data.insert(data.end(), fdata, fdata+16);
      framesize -= 4;
      data.insert(data.end(), old_framedata, old_framedata+framesize);
    }
    return true;
  }
  SB_LOG(ERROR) << "AddVP9Header Wrong frame_number:" << frame_number << " size:" << dsize;
  return false;
}

#if defined(COBALT_WIDEVINE_OPTEE)
void AmlAVCodec::CopyClearBufferToSecure(InputBuffer *input_buffer) {
  SB_DCHECK(codec_param->drmmode);
  int size = input_buffer->size();
  frame_data.assign(sizeof(drminfo_t), 0);
  drminfo_t *drminfo = (drminfo_t *)&frame_data[0];
  drminfo->drm_level = DRM_LEVEL1;
  drminfo->drm_pktsize = size;
#if 1
  drminfo->drm_hasesdata = 1;
  if (codec_param->video_type == VFORMAT_VP9) {
    std::vector<uint8_t> amldata;
    if (AddVP9Header(const_cast<uint8_t *>(input_buffer->data()), size,
                     amldata)) {
      drminfo->drm_pktsize = amldata.size();
      frame_data.insert(frame_data.end(), amldata.begin(), amldata.end());
    }
  } else {
    frame_data.insert(frame_data.end(), input_buffer->data(),
                      input_buffer->data() + size);
  }
#else
  uint8_t *phy = GetSecMem(size);
  drminfo->drm_hasesdata = 0;
  drminfo->drm_phy = (unsigned int)((size_t)phy);
  drminfo->drm_flag = TYPE_DRMINFO | TYPE_PATTERN;
#if 1
  ::starboard::shared::widevine::DrmSystemWidevine::CopyBuffer(phy, input_buffer->data(), size);
#else
  uint8_t *secdata = (uint8_t *)(sec_drm_mem_virt - sec_drm_mem) +
                     (sec_drm_mem_off + drminfo->drm_phy);
  SbMemoryCopy(secdata, input_buffer->data(), size);
  CLOG(ERROR) << "copy to secmem size " << size
              << " phy:" << (void *)drminfo->drm_phy
              << " vir:" << (void *)secdata;
#endif
#endif
  last_pts_in_secure = input_buffer->timestamp();
  input_buffer->SetDecryptedContent(&frame_data[0], frame_data.size());
}

bool AmlAVCodec::IsSampleInSecureBuffer(InputBuffer *input_buffer)
{
    return (last_pts_in_secure==input_buffer->timestamp());
}
#endif


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
  pts_sb = input_buffer->timestamp();
  if (input_buffer->drm_info()) {
    if (!SbDrmSystemIsValid(drm_system_)) {
      return false;
    }
    SbDrmSystemPrivate::DecryptStatus decrypt_status =
        drm_system_->Decrypt(input_buffer);
    if (decrypt_status == SbDrmSystemPrivate::kRetry) {
      *written = false;
      return true;
    }
    if (decrypt_status == SbDrmSystemPrivate::kFailure) {
      *written = false;
      return false;
    }
#if defined(COBALT_WIDEVINE_OPTEE)
    if (codec_param->drmmode && input_buffer->drm_info()) {
      CopyClearBufferToSecure(input_buffer);
    }
#endif
    data = const_cast<uint8_t *>(input_buffer->data());
    size = input_buffer->size();
    last_pts_in_secure = input_buffer->timestamp();
  }
#if defined(COBALT_WIDEVINE_OPTEE)
  else if (codec_param->drmmode && !IsSampleInSecureBuffer(input_buffer)) {
    CopyClearBufferToSecure(input_buffer);
    data = const_cast<uint8_t *>(input_buffer->data());
    size = input_buffer->size();
  }
#endif
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
  CLOG(WARNING) << "audio format_tag:" << audio_header.format_tag
              << " number_of_channels:" << audio_header.number_of_channels
              << " samples_per_second:" << audio_header.samples_per_second
              << " average_bytes_per_second:" << audio_header.average_bytes_per_second
              << " block_alignment:" << audio_header.block_alignment
              << " bits_per_sample:" << audio_header.bits_per_sample
              << " audio_specific_config_size:"
              << audio_header.audio_specific_config_size;
  drm_system_ = drm_system;
  if (audio_codec == kSbMediaAudioCodecAac) {
    codec_param->audio_type = AFORMAT_AAC;
  } else if (audio_codec == kSbMediaAudioCodecOpus) {
    codec_param->audio_type = AFORMAT_OPUS;
    //codec_param->audio_info.valid = 1;
    //codec_param->audio_info.sample_rate = audio_header.samples_per_second;
    //codec_param->audio_info.channels = audio_header.number_of_channels;
    //codec_param->audio_info.codec_id = 0x4f505553;  // OPUS
    codec_param->audio_channels = audio_header.number_of_channels;
    codec_param->audio_samplerate = audio_header.samples_per_second;
    feed_data_func = std::bind(&AmlAudioRenderer::WriteOpusSample, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  } else {
    CLOG(ERROR) << "can't support audio codecs other than AAC and Opus";
    codec_param->audio_type = AFORMAT_AAC;
  }
  codec_param->audio_info.sample_rate = audio_header.samples_per_second;
  codec_param->audio_info.channels = audio_header.number_of_channels;
  //codec_param->audio_info.bitrate = audio_header.samples_per_second * audio_header.bits_per_sample;
  //codec_param->audio_info.block_align = audio_header.block_alignment;
  codec_param->audio_info.extradata_size = audio_header.audio_specific_config_size;
  SbMemoryCopy(codec_param->audio_info.extradata, audio_header.audio_specific_config, audio_header.audio_specific_config_size);
  codec_param->stream_type = STREAM_TYPE_ES_AUDIO;
  codec_param->has_audio = 1;
  codec_param->has_video = 1;
  AVInitCodec();
}

bool AmlAudioRenderer::WriteOpusSample(uint8_t * buf, int dsize, bool *written)
{
  frame_data.resize(2+dsize);
  frame_data[0] = dsize & 0xff;
  frame_data[1] = (dsize >> 8)&0xff;
  SbMemoryCopy(&frame_data[2], buf, dsize);
  return WriteCodec(&frame_data[0], frame_data.size(), written);
}

AmlVideoRenderer::AmlVideoRenderer(SbMediaVideoCodec video_codec,
                                   SbDrmSystem drm_system,
                                   SbPlayerOutputMode output_mode,
                                   SbDecodeTargetGraphicsContextProvider
                                       *decode_target_graphics_context_provider) {
  drm_system_ = drm_system;
  codec_param->has_video = 1;
#if defined(COBALT_WIDEVINE_OPTEE)
  if (drm_system_) {
    codec_param->drmmode = 1;
    if (sec_drm_mem) {
      CLOG(ERROR) << "unexpected: Secure memory shouldn't be inialized here";
      Secure_ReleaseResource();
      sec_drm_mem = NULL;
    }
    bool is_4k = true;
    sec_drm_mem = (uint8_t *)Secure_AllocSecureMem(
        sec_mem_size,
        (is_4k ? 2 : 1) | ((video_codec == kSbMediaVideoCodecVp9) ? 0x10 : 0));
    sec_drm_mem_virt = NULL;
#if 1
    int fd = open("/dev/mem", O_RDWR);
    if (fd > 0) {
      sec_drm_mem_off = (size_t)sec_drm_mem & (sysconf(_SC_PAGE_SIZE) - 1);
      sec_drm_mem_virt =
          (uint8_t*)mmap(NULL, sec_mem_size + sec_drm_mem_off, PROT_READ | PROT_WRITE,
               MAP_SHARED, fd, (size_t)sec_drm_mem - sec_drm_mem_off);
      close(fd);
    }
#endif
    sec_mem_pos = 0;
    (static_cast<::starboard::shared::widevine::DrmSystemWidevine*>(drm_system_))->AttachDecoder(this);
    CLOG(WARNING) << "Enable TVP, sec memory " << (void*)sec_drm_mem << ", mapped address " << (void*)sec_drm_mem_virt << ", offset " << sec_drm_mem_off;
    frame_data.resize(sizeof(drminfo_t));
  }
#endif
  if (video_codec == kSbMediaVideoCodecVp9) {
    codec_param->video_type = VFORMAT_VP9;
    codec_param->am_sysinfo.format = VIDEO_DEC_FORMAT_VP9;
#if defined(COBALT_WIDEVINE_OPTEE)
    if (codec_param->drmmode)
      feed_data_func = std::bind(&AmlVideoRenderer::WriteVP9SampleTvp, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3);
    else
      feed_data_func = std::bind(&AmlVideoRenderer::WriteVP9Sample, this,
                                 std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3);
#else
    feed_data_func = std::bind(&AmlVideoRenderer::WriteVP9Sample, this,
                               std::placeholders::_1, std::placeholders::_2,
                               std::placeholders::_3);
#endif
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
  if (AddVP9Header(buf, dsize, frame_data) && !frame_data.empty()) {
    return WriteCodec(&frame_data[0], frame_data.size(), written);
  }
  return false;
}

#if defined(COBALT_WIDEVINE_OPTEE)
bool AmlVideoRenderer::WriteVP9SampleTvp(uint8_t *buf, int dsize,
                                         bool *written) {
  SB_DCHECK(codec_param->drmmode);
  SB_DCHECK(dsize >= sizeof(drminfo_t));
  if (dsize > sizeof(drminfo_t)) { // clear data
    drminfo_t *drminfo = (drminfo_t *)buf;
    if ((drminfo->drm_hasesdata != 1) ||
        (drminfo->drm_pktsize + sizeof(drminfo_t) != dsize)) {
      CLOG(ERROR) << "TVP data error drminfo:{drm_level=" << drminfo->drm_level
                  << ",drm_pktsize=" << drminfo->drm_pktsize
                  << ",drm_hasesdata=" << drminfo->drm_hasesdata
                  << ",drm_phy=" << (void *)drminfo->drm_phy
                  << ",drm_flag=" << drminfo->drm_flag
                  << "}, total size:" << dsize;
      return false;
    }
    return WriteCodec(buf, dsize, written);
  }
  unsigned int header_size = 0;
  frame_data.assign(buf, buf + dsize);
  drminfo_t *drminfo = (drminfo_t *)&frame_data[0];
  Secure_GetVp9HeaderSize((void *)drminfo->drm_phy, drminfo->drm_pktsize,
                          &header_size);
  drminfo->drm_pktsize += header_size;
  return WriteCodec(&frame_data[0], frame_data.size(), written);
}
#endif

} // namespace filter
} // namespace player
} // namespace starboard
} // namespace shared
} // namespace starboard
