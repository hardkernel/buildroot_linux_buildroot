#ifndef THIRD_PARTY_STARBOARD_AMLOGIC_SHARED_AML_AV_COMPONENTS_H
#define THIRD_PARTY_STARBOARD_AMLOGIC_SHARED_AML_AV_COMPONENTS_H

#include <algorithm>

extern "C" {
#include <amthreadpool.h>
#include <codec.h>
#include <Amavutils.h>
}

#include "starboard/memory.h"
#include "starboard/common/scoped_ptr.h"
#include "starboard/common/ref_counted.h"
#include "starboard/shared/starboard/media/media_util.h"
#include "starboard/shared/starboard/player/job_queue.h"
#include "starboard/shared/starboard/player/filter/callback.h"
#include "starboard/shared/starboard/player/filter/media_time_provider.h"
#include "starboard/shared/starboard/player/input_buffer_internal.h"


namespace starboard {
namespace shared {
namespace starboard {
namespace player {
namespace filter {

class AmlAVCodec : public MediaTimeProvider, public JobQueue::JobOwner {
public:
#define CLOG(level) SB_LOG(level) << name
  AmlAVCodec();
  virtual ~AmlAVCodec();
  void AVInitialize(const ErrorCB &error_cb, const PrerolledCB &prerolled_cb,
                    const EndedCB &ended_cb);
  bool AVInitCodec();
  bool AVWriteSample(const scoped_refptr<InputBuffer> &input_buffer,
                     bool *written);
  void AVWriteEndOfStream();
  bool AVIsEndOfStreamWritten() const;
  void AVSetVolume(double volume);
  void AVPlay();
  void AVPause();
  void AVSetPlaybackRate(double playback_rate);
  void AVSeek(SbTime seek_to_time);
  int AVGetDroppedFrames() const;
  SbTime AVGetCurrentMediaTime(bool *is_playing, bool *is_eos_played);

  bool WriteCodec(uint8_t * buf, int dsize, bool *written);

  // MediaTimeProvider methods
  void Initialize(const ErrorCB &error_cb, const PrerolledCB &prerolled_cb,
                  const EndedCB &ended_cb) override {
    AVInitialize(error_cb, prerolled_cb, ended_cb);
  }
  void Play() override { AVPlay(); }
  void Pause() override { AVPause(); }
  void SetPlaybackRate(double playback_rate) override {
    AVSetPlaybackRate(playback_rate);
  }
  void Seek(SbTime seek_to_time) override { AVSeek(seek_to_time); }
  SbTime GetCurrentMediaTime(bool *is_playing, bool *is_eos_played, bool* is_underflow) override {
    return AVGetCurrentMediaTime(is_playing, is_eos_played);
  }

protected:
  void AVCheckDecoderEos();
  std::string name;
  codec_para_t *codec_param;
  ErrorCB error_cb_;
  PrerolledCB prerolled_cb_;
  EndedCB ended_cb_;
  std::function<void()> func_check_eos;
  std::function<bool(uint8_t *, int, bool*)> feed_data_func;
  std::vector<uint8_t> frame_data;
  int eos_state; // 0: no eos, 1:eos but play buffering data, 2:eos and no more
                 // data in buffer
  unsigned int last_read_point;
  SbTime rp_freeze_time;
  bool isvideo;
  bool prerolled;
};

class AmlAudioRenderer : public AmlAVCodec {
public:
  AmlAudioRenderer(SbMediaAudioCodec audio_codec,
                   SbMediaAudioHeader audio_header, SbDrmSystem drm_system);
  virtual ~AmlAudioRenderer() {}
  // AudioRenderer methods
  bool WriteSample(const scoped_refptr<InputBuffer> &input_buffer,
                   bool *written) {
    return AVWriteSample(input_buffer, written);
  }
  void WriteEndOfStream() { AVWriteEndOfStream(); }
  void SetVolume(double volume) { AVSetVolume(volume); }
  bool IsEndOfStreamWritten() const { return AVIsEndOfStreamWritten(); }
};

class AmlVideoRenderer: public AmlAVCodec {
public:
  AmlVideoRenderer(SbMediaVideoCodec video_codec, SbDrmSystem drm_system,
                   SbPlayerOutputMode output_mode,
                   SbDecodeTargetGraphicsContextProvider
                       *decode_target_graphics_context_provider);
  virtual ~AmlVideoRenderer() {}
  void Initialize(const ErrorCB &error_cb, const PrerolledCB &prerolled_cb,
                  const EndedCB &ended_cb) {
    AVInitialize(error_cb, prerolled_cb, ended_cb);
  }
  int GetDroppedFrames() const { return AVGetDroppedFrames(); }
  void Seek(SbTime seek_to_time) { AVSeek(seek_to_time); }
  bool WriteSample(const scoped_refptr<InputBuffer> &input_buffer,
                   bool *written) {
    return AVWriteSample(input_buffer, written);
  }
  void WriteEndOfStream() { AVWriteEndOfStream(); }
  bool IsEndOfStreamWritten() const { return AVIsEndOfStreamWritten(); }

  // Both of the following two functions can be called on any threads.
  void SetBounds(int z_index, int x, int y, int width, int height);
  SbDecodeTarget GetCurrentDecodeTarget();

  bool WriteVP9Sample(uint8_t * buf, int dsize, bool *written);
  int bound_x;
  int bound_y;
  int bound_w;
  int bound_h;
};

} // namespace filter
} // namespace player
} // namespace starboard
} // namespace shared
} // namespace starboard
#endif //THIRD_PARTY_STARBOARD_AMLOGIC_SHARED_AML_AV_COMPONENTS_H
