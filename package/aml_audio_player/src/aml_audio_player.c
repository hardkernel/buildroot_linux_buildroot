/*
 * Copyright (c) 2012 Stefano Sabatini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * Demuxing and decoding example.
 *
 * Show how to use the libavformat and libavcodec API to demux and
 * decode audio and video data.
 * @example demuxing_decoding.c
 */

#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>

#include <pthread.h>
#include <unistd.h>

#include "aml_audio_player.h"

//#define AML_DEBUG
#define AML_TEST

static AVStream *audio_stream = NULL;
static int audio_stream_idx = -1;
static AVFrame *frame = NULL;
static AVPacket pkt;
static AMLAudioPlayerCallback callback_func = NULL;
// static int audio_frame_count = 0;

#define PrintErr(...)                                                          \
  {                                                                            \
    fprintf(stderr, "File:%s Line:%d, Func:%s ==> ", __FILE__, __LINE__,       \
            __func__);                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
  }
#define SAINT_CHECK(X)                                                         \
  if (!(X)) {                                                                  \
    PrintErr("Assert (%s)\n", #X);                                             \
    exit(1);                                                                   \
  }

typedef enum {
  AML_PLAYER_UNKNOWN,
  AML_PLAYER_INIT,
  AML_PLAYER_OPEN,
  AML_PLAYER_PLAY,
  AML_PLAYER_PAUSE,
  AML_PLAYER_CLOSE,
} AMLAudioPlayerStatus_e;

typedef enum {
  AML_DECODER_UNKNOWN,
  AML_DECODER_PLAY,
  AML_DECODER_PAUSE,
  AML_DECODER_STOP,
} AMLAudioDecoderStatus_e;

typedef enum {
  AML_DEC_DONE,
  AML_DEC_START,
  AML_DEC_PAUSE,
  AML_DEC_RESUME,
  AML_DEC_STOP,
} AMLAudioDecoderCmd_e;

typedef struct _tag_aml_audio_output_config {
  enum AVSampleFormat out_format; //=AV_SAMPLE_FMT_S16; //AV_SAMPLE_FMT_FLT;
  enum AVCodecID out_codec;       // = AV_CODEC_ID_PCM_S16LE;
  int out_channels;               // = 2,
  int out_samples;                // = 512,
  int sample_rate;                // = 44100;
  int out_max_buffer_size;
  char out_dev[32];
} AMLAudioOutputConfig_t;

typedef struct _tag_aml_audio_player_handler {
  pthread_mutex_t decoder_mutex;
  pthread_cond_t decoder_cond;
  pthread_t player_pthread_id;

  AMLAudioPlayerStatus_e state;
  AMLAudioDecoderStatus_e decoder_state;
  AMLAudioDecoderCmd_e DecCmd;

  // Decoder related struct
  AVFormatContext *out_fmt_ctx;
  AVFormatContext *in_fmt_ctx;
  AVStream *out_stream;
  AVCodecContext *pCodecCtx;
  SwrContext *swr_ctx;
  AMLAudioOutputConfig_t output_cfg;

  uint8_t *out_buffer;
  char URL[256];

} AMLAudioPlayerHandler_t;

static AMLAudioPlayerHandler_t AMLAudioPlayer = {
  state : AML_PLAYER_UNKNOWN,
};

static char *AMLPlayerStateStr(int state) {
  char *ret = NULL;
  switch (state) {
  case AML_PLAYER_INIT:
    ret = "PLAYER_INIT";
    break;
  case AML_PLAYER_OPEN:
    ret = "PLAYER_OPEN";
    break;
  case AML_PLAYER_PLAY:
    ret = "PLAYER_PLAY";
    break;
  case AML_PLAYER_PAUSE:
    ret = "PLAYER_PAUSE";
    break;
  case AML_PLAYER_CLOSE:
    ret = "PLAYER_CLOSE";
    break;
  case AML_PLAYER_UNKNOWN:
    ret = "PLAYER_UNKNOWN";
    break;
  default:
    ret = "PLAYER_UNDEF";
    break;
  }
  return ret;
}

static char *AMLDecoderStateStr(int state) {
  char *ret = NULL;
  switch (state) {
  case AML_DECODER_UNKNOWN:
    ret = "DEC_UNKNOWN";
    break;
  case AML_DECODER_PLAY:
    ret = "DEC_PLAY";
    break;
  case AML_DECODER_PAUSE:
    ret = "DEC_PAUSE";
    break;
  case AML_DECODER_STOP:
    ret = "DEC_STOP";
    break;
  default:
    ret = "DEC_UNDEF";
    break;
  }
  return ret;
}

static void GetTimedOutInfo(int TimedOut, struct timespec *pRetTime) {
  int left, diff;
  struct timespec ots;

  clock_gettime(CLOCK_REALTIME, &ots);
  *pRetTime = ots;
  pRetTime->tv_sec += TimedOut / 1000;

  left = TimedOut % 1000;
  left *= 1000000;
  diff = 1000000000 - ots.tv_nsec;

  if (diff <= left) {
    pRetTime->tv_sec++;
    pRetTime->tv_nsec = left - diff;
  } else {
    pRetTime->tv_nsec += left;
  }
  return;
}

static int AMLAudioPlayer_WaitForDecoderState(int TimedOut,
                                              AMLAudioDecoderStatus_e State) {
  int ret = 0;
  struct timespec wts;

  GetTimedOutInfo(TimedOut, &wts);

  //	PrintErr("Conditional Wait start\n");
  while ((ETIMEDOUT != ret) && (AMLAudioPlayer.decoder_state != State)) {
    pthread_mutex_lock(&AMLAudioPlayer.decoder_mutex);
    ret = pthread_cond_timedwait(&AMLAudioPlayer.decoder_cond,
                                 &AMLAudioPlayer.decoder_mutex, &wts);
    pthread_mutex_unlock(&AMLAudioPlayer.decoder_mutex);
    if (ETIMEDOUT != ret && (AMLAudioPlayer.decoder_state != State)) {
      //			PrintErr("ConditionalTriggered, but not wanted
      //decoder status: %s, request:%s\n", AMLAudioPlayer.decoder_state, State);
      continue;
    }
    //		if(ETIMEDOUT == ret)
    //			PrintErr("wait: TimedOut\n")
  }
  //	PrintErr("Conditional Wait End\n");
  return AMLAudioPlayer.decoder_state == State;
}

static int AMLAudioPlayer_WaitForDecoderCmd(int TimedOut,
                                            AMLAudioDecoderCmd_e Cmd1,
                                            AMLAudioDecoderCmd_e Cmd2) {
  int ret = 0;
  struct timespec wts;

  GetTimedOutInfo(TimedOut, &wts);

  //	PrintErr("Conditional Wait start\n");
  while ((ETIMEDOUT != ret) && (AMLAudioPlayer.DecCmd != Cmd1) &&
         (AMLAudioPlayer.DecCmd != Cmd2)) {
    pthread_mutex_lock(&AMLAudioPlayer.decoder_mutex);
    ret = pthread_cond_timedwait(&AMLAudioPlayer.decoder_cond,
                                 &AMLAudioPlayer.decoder_mutex, &wts);
    pthread_mutex_unlock(&AMLAudioPlayer.decoder_mutex);
    if (ETIMEDOUT != ret && (AMLAudioPlayer.DecCmd != Cmd1) &&
        (AMLAudioPlayer.DecCmd != Cmd2)) {
      //			PrintErr("ConditionalTriggered, but not wanted
      //decoder cmd: %d, request:%d\n", AMLAudioPlayer.DecCmd, Cmd1, Cmd2);
      continue;
    }
    //		if(ETIMEDOUT == ret)
    //			PrintErr("wait: TimedOut\n")
  }
  //	PrintErr("Conditional Wait End\n");
  return AMLAudioPlayer.DecCmd == Cmd1 || AMLAudioPlayer.DecCmd == Cmd2;
}

/* Enable or disable frame reference counting. You are not supposed to support
 * both paths in your application but pick the one most appropriate to your
 * needs. Look for the use of refcount in this example to see what are the
 * differences of API usage between them. */
static int refcount = 0;

static int open_output_context(void) {
  AMLAudioOutputConfig_t *pAudioOutCfg = &(AMLAudioPlayer.output_cfg);

  // allocate empty format context
  // provides methods for writing output packets
  AMLAudioPlayer.out_fmt_ctx = avformat_alloc_context();
  if (!AMLAudioPlayer.out_fmt_ctx) {
    PrintErr("avformat_alloc_context()\n");
    return AML_ERR_PLAYER_INT;
  }

  // find output format for ALSA device
  AVOutputFormat *fmt = av_guess_format("alsa", NULL, NULL);
  if (!fmt) {
    PrintErr("av_guess_format()\n");
    return AML_ERR_OUTPUT;
  }

  // tell format context to use ALSA as ouput device
  AMLAudioPlayer.out_fmt_ctx->oformat = fmt;

  // add stream to format context
  AMLAudioPlayer.out_stream =
      avformat_new_stream(AMLAudioPlayer.out_fmt_ctx, NULL);
  if (!AMLAudioPlayer.out_stream) {
    PrintErr("avformat_new_stream()\n");
    return AML_ERR_OUTPUT;
  }

  // Following parameter are configured by Player_Init();
  //	  pAudioOutCfg->out_channels = 2;
  //	  pAudioOutCfg->out_format = AV_SAMPLE_FMT_S16;
  //	  pAudioOutCfg->sample_rate = 44100;
  pAudioOutCfg->out_codec = AV_CODEC_ID_PCM_S16LE;
  pAudioOutCfg->out_samples = 512;
  pAudioOutCfg->out_max_buffer_size = av_samples_get_buffer_size(
      NULL, pAudioOutCfg->out_channels, pAudioOutCfg->out_samples,
      pAudioOutCfg->out_format, 1);

  // allocate buffer for output stream
  // out_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
  AMLAudioPlayer.out_buffer =
      (uint8_t *)av_malloc(pAudioOutCfg->out_max_buffer_size);
  SAINT_CHECK(AMLAudioPlayer.out_buffer);

  // initialize converter from input audio stream to output stream
  // provides methods for converting decoded packets to output stream
  AMLAudioPlayer.swr_ctx = swr_alloc_set_opts(
      NULL,
      av_get_default_channel_layout(pAudioOutCfg->out_channels), // output
      pAudioOutCfg->out_format,                                  // output
      pAudioOutCfg->sample_rate,                                 // output
      // FIX:Some Codec's Context Information is missing
      av_get_default_channel_layout(
          AMLAudioPlayer.pCodecCtx->channels), // input
      AMLAudioPlayer.pCodecCtx->sample_fmt,    // input
      AMLAudioPlayer.pCodecCtx->sample_rate,   // input
      0, NULL);
  if (!AMLAudioPlayer.swr_ctx) {
    PrintErr("error: swr_alloc_set_opts()\n");
    return AML_ERR_OUTPUT;
  }
  swr_init(AMLAudioPlayer.swr_ctx);

  AVCodecParameters *out_codec_ctx = AMLAudioPlayer.out_stream->codecpar;
  SAINT_CHECK(out_codec_ctx);

  out_codec_ctx->codec_id = pAudioOutCfg->out_codec;
  out_codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
  out_codec_ctx->format = pAudioOutCfg->out_format;
  out_codec_ctx->bit_rate = 0;
  out_codec_ctx->sample_rate = pAudioOutCfg->sample_rate;
  out_codec_ctx->channels = pAudioOutCfg->out_channels;
  out_codec_ctx->channel_layout =
      av_get_default_channel_layout(pAudioOutCfg->out_channels);

  printf("Codec Info: Bitrate:%lld, SampleRate:%d, Channels:%d, Layout:%lld, "
         "format:%d\n",
         AMLAudioPlayer.pCodecCtx->bit_rate,
         AMLAudioPlayer.pCodecCtx->sample_rate,
         AMLAudioPlayer.pCodecCtx->channels,
         AMLAudioPlayer.pCodecCtx->channel_layout,
         AMLAudioPlayer.pCodecCtx->sample_fmt);

  printf("Output Info: Bitrate:%lld, SampleRate:%d, Channels:%d, Layout:%lld, "
         "format:%d\n",
         out_codec_ctx->bit_rate, out_codec_ctx->sample_rate,
         out_codec_ctx->channels, out_codec_ctx->channel_layout,
         out_codec_ctx->format);

  strcpy(AMLAudioPlayer.out_fmt_ctx->filename, pAudioOutCfg->out_dev);

  // initialze output device
  if (avformat_write_header(AMLAudioPlayer.out_fmt_ctx, NULL) < 0) {
    PrintErr("avformat_write_header()\n");
    return AML_ERR_OUTPUT;
  }

  return AML_NO_ERR;
}

static int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx,
                              AVFormatContext *fmt_ctx, enum AVMediaType type) {
  int ret, stream_index;
  AVStream *st;
  AVCodec *dec = NULL;
  AVDictionary *opts = NULL;

  ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
  if (ret < 0) {
    PrintErr("Could not find %s stream in input file '%s'\n",
             av_get_media_type_string(type), AMLAudioPlayer.URL);
    return ret;
  } else {
    stream_index = ret;
    st = fmt_ctx->streams[stream_index];

    /* find decoder for the stream */
    dec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!dec) {
      PrintErr("Failed to find %s codec\n", av_get_media_type_string(type));
      return AVERROR(EINVAL);
    }

    /* Allocate a codec context for the decoder */
    *dec_ctx = avcodec_alloc_context3(dec);
    if (!*dec_ctx) {
      PrintErr("Failed to allocate the %s codec context\n",
               av_get_media_type_string(type));
      return AVERROR(ENOMEM);
    }

    /* Copy codec parameters from input stream to output codec context */
    if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
      PrintErr("Failed to copy %s codec parameters to decoder context\n",
               av_get_media_type_string(type));
      return ret;
    }

    /* Init the decoders, with or without reference counting */
    av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
    if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
      PrintErr("Failed to open %s codec\n", av_get_media_type_string(type));
      return ret;
    }
    *stream_idx = stream_index;
  }

  return 0;
}

#if 0
static int get_format_from_sample_fmt(const char **fmt,
        enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}
#endif

static int decode_packet(int *got_frame, int cached) {
  int ret = 0;
  int decoded = pkt.size;
  uint8_t *out_buffer = NULL;
  int data_size = 0;

  *got_frame = 0;

  if (pkt.stream_index == audio_stream_idx) {

#if 1
    ret = avcodec_send_packet(AMLAudioPlayer.pCodecCtx, &pkt);
    if (ret < 0) {
      // PrintErr("Error decoding audio frame (%s)\n", av_err2str(ret));
      PrintErr("Error decoding audio frame, ret:%d\n", ret);
      return ret;
    }

    while (ret >= 0) {
      ret = avcodec_receive_frame(AMLAudioPlayer.pCodecCtx, frame);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        break;
      else if (ret < 0) {
        PrintErr("Error decoding audio frame, ret:%d\n", ret);
        return ret;
      }

      data_size = av_get_bytes_per_sample(AMLAudioPlayer.pCodecCtx->sample_fmt);
      if (data_size < 0) {
        /* This should not occur, checking just for paranoia */
        PrintErr("Failed to calculate data size\n");
        return AML_ERR_PLAYER_INT;
      }

      if (!AMLAudioPlayer.out_buffer) {
        ret = open_output_context(); // Delay output init util first frame is
                                     // ready;
        if (ret < 0) {
          PrintErr("Failed to open alsa output\n");
          return ret;
        }
      }

      out_buffer = AMLAudioPlayer.out_buffer;

      if (data_size > 0 && frame->nb_samples > 0) {
        int got_samples =
            swr_convert(AMLAudioPlayer.swr_ctx, &out_buffer,
                        AMLAudioPlayer.output_cfg.out_samples,
                        (const uint8_t **)frame->data, frame->nb_samples);

        if (got_samples < 0) {
          PrintErr("error: swr_convert()\n");
          return AML_ERR_PLAYER_INT;
        }

        //  printf("output to alsa samples:%d\n", got_samples);
        while (got_samples > 0) {
          int buffer_size = av_samples_get_buffer_size(
              NULL, AMLAudioPlayer.output_cfg.out_channels, got_samples,
              AMLAudioPlayer.output_cfg.out_format, 1);
          SAINT_CHECK(buffer_size <=
                      AMLAudioPlayer.output_cfg.out_max_buffer_size);

          // create output packet
          AVPacket out_packet;
          av_init_packet(&out_packet);
          out_packet.data = out_buffer;
          out_packet.size = buffer_size;

          // write output packet to format context
          if (av_write_frame(AMLAudioPlayer.out_fmt_ctx, &out_packet) < 0) {
            PrintErr("av_write_frame()\n");
            return AML_ERR_PLAYER_INT;
          }

          // process samples buffered inside swr context
          got_samples =
              swr_convert(AMLAudioPlayer.swr_ctx, &out_buffer,
                          AMLAudioPlayer.output_cfg.out_samples, NULL, 0);
          if (got_samples < 0) {
            PrintErr("error: swr_convert()\n");
            return AML_ERR_PLAYER_INT;
          }
          // printf("2_output to alsa samples:%d\n", got_samples);
        }
      }
    }
#else
    /* decode audio frame */
    ret =
        avcodec_decode_audio4(AMLAudioPlayer.pCodecCtx, frame, got_frame, &pkt);
    // printf("avcodec_decode_audio4: pkt.size:%d PTS:%lld, ret:%d,
    // got_frame:%d\n", pkt.size, pkt.pts, ret, *got_frame);
    if (ret < 0) {
      // PrintErr("Error decoding audio frame (%s)\n", av_err2str(ret));
      PrintErr("Error decoding audio frame, ret:%d\n", ret);
      return ret;
    }
    /* Some audio decoders decode only part of the packet, and have to be
     * called again with the remainder of the packet data.
     * Sample: fate-suite/lossless-audio/luckynight-partial.shn
     * Also, some decoders might over-read the packet. */
    decoded = FFMIN(ret, pkt.size);
#if 0
        if (*got_frame) {
            size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
            printf("audio_frame%s n:%d nb_samples:%d pts:%s\n",
                    cached ? "(cached)" : "",
                    audio_frame_count++, frame->nb_samples,
                    av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

            /* Write the raw audio data samples of the first plane. This works
             * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
             * most audio decoders output planar audio, which uses a separate
             * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
             * In other words, this code will write only the first audio channel
             * in these cases.
             * You should use libswresample or libavfilter to convert the frame
             * to packed data. */
            fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);
        }
#endif
    if (*got_frame) {

      if (!AMLAudioPlayer.out_buffer) {
        ret = open_output_context(); // Delay output init util first frame is
                                     // ready;
        if (ret < 0) {
          PrintErr("Failed to open alsa output\n");
          return ret;
        }
      }

      int data_size = 0;
      data_size = av_get_bytes_per_sample(AMLAudioPlayer.pCodecCtx->sample_fmt);
      if (data_size < 0) {
        /* This should not occur, checking just for paranoia */
        PrintErr("Failed to calculate data size\n");
        return AML_ERR_PLAYER_INT;
      }
      out_buffer = AMLAudioPlayer.out_buffer;

      if (data_size > 0 && frame->nb_samples > 0) {
        int got_samples =
            swr_convert(AMLAudioPlayer.swr_ctx, &out_buffer,
                        AMLAudioPlayer.output_cfg.out_samples,
                        (const uint8_t **)frame->data, frame->nb_samples);

        if (got_samples < 0) {
          PrintErr("error: swr_convert()\n");
          return AML_ERR_PLAYER_INT;
        }

        //  printf("output to alsa samples:%d\n", got_samples);
        while (got_samples > 0) {
          int buffer_size = av_samples_get_buffer_size(
              NULL, AMLAudioPlayer.output_cfg.out_channels, got_samples,
              AMLAudioPlayer.output_cfg.out_format, 1);
          SAINT_CHECK(buffer_size <=
                      AMLAudioPlayer.output_cfg.out_max_buffer_size);

          // create output packet
          AVPacket out_packet;
          av_init_packet(&out_packet);
          out_packet.data = out_buffer;
          out_packet.size = buffer_size;

          // write output packet to format context
          if (av_write_frame(AMLAudioPlayer.out_fmt_ctx, &out_packet) < 0) {
            PrintErr("av_write_frame()\n");
            return AML_ERR_PLAYER_INT;
          }

          // process samples buffered inside swr context
          got_samples =
              swr_convert(AMLAudioPlayer.swr_ctx, &out_buffer,
                          AMLAudioPlayer.output_cfg.out_samples, NULL, 0);
          if (got_samples < 0) {
            PrintErr("error: swr_convert()\n");
            return AML_ERR_PLAYER_INT;
          }
          // printf("2_output to alsa samples:%d\n", got_samples);
        }
      }
    }
#endif
  }

  /* If we use frame reference counting, we own the data and need
   * to de-reference it when we don't use it anymore */
  if (*got_frame && refcount)
    av_frame_unref(frame);

  return decoded;
}

static void *AMLAudioPlayer_decoder_thread(void *arg) {

  int ret, got_frame;
  int need_notify_eof = 0;

  pthread_detach(pthread_self());

  /* initialize packet, set data to NULL, let the demuxer fill it */
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;

  while (AMLAudioPlayer.state != AML_PLAYER_UNKNOWN) {

    if (AMLAudioPlayer.DecCmd != AML_DEC_START) {
      AMLAudioPlayer_WaitForDecoderCmd(5000, AML_DEC_START, AML_DEC_START);
      //			sleep(1);
      //			printf("decoder thread is waiting for start,
      //decoder_status:%d\n", AMLAudioPlayer.decoder_state);
      continue;
    }

    AMLAudioPlayer.decoder_state = AML_DECODER_PLAY;
    AMLAudioPlayer.DecCmd = AML_DEC_DONE;
    pthread_cond_signal(&AMLAudioPlayer.decoder_cond);

    /* read frames from the file */
    while (AMLAudioPlayer.decoder_state != AML_DECODER_STOP) {
      if (AMLAudioPlayer.decoder_state == AML_DECODER_PLAY) {
        if (AMLAudioPlayer.DecCmd == AML_DEC_PAUSE) {
          AMLAudioPlayer.decoder_state = AML_DECODER_PAUSE;
          AMLAudioPlayer.DecCmd = AML_DEC_DONE;
          pthread_cond_signal(&AMLAudioPlayer.decoder_cond);
          continue;
        }

        if (AMLAudioPlayer.DecCmd == AML_DEC_STOP) {
          AMLAudioPlayer.decoder_state = AML_DECODER_STOP;
          AMLAudioPlayer.DecCmd = AML_DEC_DONE;
          pthread_cond_signal(&AMLAudioPlayer.decoder_cond);
          continue;
        }
      }

      if (AMLAudioPlayer.decoder_state == AML_DECODER_PAUSE) {
        if (AMLAudioPlayer.DecCmd == AML_DEC_RESUME) {
          AMLAudioPlayer.decoder_state = AML_DECODER_PLAY;
          AMLAudioPlayer.DecCmd = AML_DEC_DONE;
          pthread_cond_signal(&AMLAudioPlayer.decoder_cond);
          continue;
        }

        if (AMLAudioPlayer.DecCmd == AML_DEC_STOP) {
          AMLAudioPlayer.decoder_state = AML_DECODER_STOP;
          AMLAudioPlayer.DecCmd = AML_DEC_DONE;
          pthread_cond_signal(&AMLAudioPlayer.decoder_cond);
          continue;
        }
        AMLAudioPlayer_WaitForDecoderCmd(5000, AML_DEC_RESUME, AML_DEC_STOP);
        //	printf("decoder is in pausing, we can use conditional wait\n");
        //	sleep(1);
        continue;
      }

      ret = av_read_frame(AMLAudioPlayer.in_fmt_ctx, &pkt);
      if (ret < 0)
        break;
      else {
        AVPacket orig_pkt = pkt;
        do {
          ret = decode_packet(&got_frame, 0);
          if (ret < 0)
            break;
          pkt.data += ret;
          pkt.size -= ret;
        } while (pkt.size > 0);
        av_packet_unref(&orig_pkt);
      }
      if (ret < 0) {
        PrintErr("decode_packet return error code:%d \n", ret);
        AMLAudioPlayer.decoder_state = AML_DECODER_STOP;
        break;
      }
    }

    // No need to flush cache, if user want to stop
    if (AMLAudioPlayer.decoder_state != AML_DECODER_STOP) {
      /* flush cached frames */
      pkt.data = NULL;
      pkt.size = 0;
      do {
        decode_packet(&got_frame, 1);
      } while (got_frame);

      need_notify_eof = 1;
      PrintErr("need_notify_eof = %d\n", need_notify_eof);
    }
    AMLAudioPlayer.decoder_state = AML_DECODER_STOP;
    pthread_cond_signal(&AMLAudioPlayer.decoder_cond);
    PrintErr("Current playback is stopped\n");
    if (need_notify_eof && callback_func) {
      PrintErr("aml_audio_player call the callback\n");
      callback_func(AML_AUDIO_PLAYER_CB_EOF, NULL);
      need_notify_eof = 0;
    }
  }

  pthread_exit(NULL);
}

int AMLAudioPlayer_Install_CB(AMLAudioPlayerCallback callback) {
  PrintErr("install callback\n");
  if (NULL != callback_func) {
    PrintErr("Callback has been installed\n");
    return -1;
  }
  callback_func = callback;
  return 0;
}

int AMLAudioPlayer_Init(char *OutDev, int out_channels, int out_samplerate,
                        int out_format) {
  if (AMLAudioPlayer.state != AML_PLAYER_UNKNOWN) {
    PrintErr("Invalid player state:%s \n",
             AMLPlayerStateStr(AMLAudioPlayer.state));
    return AML_ERR_PLAYER_STATE;
  }
  // Init output param here
  AMLAudioPlayer.output_cfg.out_channels = out_channels;
  AMLAudioPlayer.output_cfg.sample_rate = out_samplerate;
  AMLAudioPlayer.output_cfg.out_format = out_format;
  strcpy(AMLAudioPlayer.output_cfg.out_dev, OutDev);

  AMLAudioPlayer.out_buffer = NULL;
  AMLAudioPlayer.out_fmt_ctx = NULL;
  AMLAudioPlayer.pCodecCtx = NULL;
  AMLAudioPlayer.in_fmt_ctx = NULL;

  /* register all formats (demux/mux) and codecs */
  av_register_all();
  // register supported devices
  avdevice_register_all();
  avformat_network_init();

  frame = av_frame_alloc();
  if (!frame) {
    PrintErr("Could not allocate frame\n");
    return AML_ERR_SYSTEM;
  }
  AMLAudioPlayer.state = AML_PLAYER_INIT;

  pthread_mutex_init(&AMLAudioPlayer.decoder_mutex, NULL);
  pthread_cond_init(&AMLAudioPlayer.decoder_cond, NULL);

  pthread_create(&AMLAudioPlayer.player_pthread_id, NULL,
                 AMLAudioPlayer_decoder_thread, NULL);

  return AML_NO_ERR;
}

int AMLAudioPlayer_DeInit(void) {
  if ((AMLAudioPlayer.state != AML_PLAYER_INIT) &&
      (AMLAudioPlayer.state != AML_PLAYER_CLOSE)) {
    PrintErr("Invalid player state:%s \n",
             AMLPlayerStateStr(AMLAudioPlayer.state));
    return AML_ERR_PLAYER_STATE;
  }

  if (frame) {
    av_frame_free(&frame);
    frame = NULL;
  }

  AMLAudioPlayer.state = AML_PLAYER_UNKNOWN;

  //	PrintErr("Wait for decoder thread quit..\n");
  pthread_cancel(AMLAudioPlayer.player_pthread_id);
  pthread_join(AMLAudioPlayer.player_pthread_id, NULL);
  //	PrintErr("Decoder thread quit\n");

  pthread_cond_destroy(&AMLAudioPlayer.decoder_cond);
  pthread_mutex_destroy(&AMLAudioPlayer.decoder_mutex);
  return AML_NO_ERR;
}

int AMLAudioPlayer_SetInput(char *URL) {
  int ret = 0;

  if ((AMLAudioPlayer.state != AML_PLAYER_INIT) &&
      (AMLAudioPlayer.state != AML_PLAYER_CLOSE)) {
    PrintErr("Invalid player state:%s \n",
             AMLPlayerStateStr(AMLAudioPlayer.state));
    return AML_ERR_PLAYER_STATE;
  }

  strcpy(AMLAudioPlayer.URL, URL);

  AVDictionary *options = NULL;
  av_dict_set_int(&options, "stimeout", (int64_t)2, 0);

  /* open input file, and allocate format context */
  if (avformat_open_input(&AMLAudioPlayer.in_fmt_ctx, AMLAudioPlayer.URL, NULL,
                          &options) < 0) {
    PrintErr("Could not open source file %s\n", AMLAudioPlayer.URL);
    return AML_ERR_INVALID_INPUT;
  }

  /* retrieve stream information */
  if (avformat_find_stream_info(AMLAudioPlayer.in_fmt_ctx, NULL) < 0) {
    PrintErr("Could not find stream information\n");
    return AML_ERR_INPUT_NO_AUDIO;
  }

  if (open_codec_context(&audio_stream_idx, &AMLAudioPlayer.pCodecCtx,
                         AMLAudioPlayer.in_fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
    audio_stream = AMLAudioPlayer.in_fmt_ctx->streams[audio_stream_idx];
  } else {
    PrintErr("open codec context return error\n");
    return AML_ERR_PLAYER_INT;
  }

#if defined(AML_DEBUG)
  /* dump input information to stderr */
  av_dump_format(AMLAudioPlayer.in_fmt_ctx, 0, AMLAudioPlayer.URL, 0);
#endif

  if (!audio_stream) {
    PrintErr("Could not find audio in the input, aborting\n");
    return AML_ERR_INPUT_NO_AUDIO;
  }
  AMLAudioPlayer.state = AML_PLAYER_OPEN;
  return ret;
}

int AMLAudioPlayer_Play(void) {
  int ret = 0;

  if (AMLAudioPlayer.state != AML_PLAYER_OPEN) {
    PrintErr("Invalid player state:%s \n",
             AMLPlayerStateStr(AMLAudioPlayer.state));
    return AML_ERR_PLAYER_STATE;
  }

  AMLAudioPlayer.DecCmd = AML_DEC_START;
  pthread_cond_signal(&AMLAudioPlayer.decoder_cond);

  if (AMLAudioPlayer_WaitForDecoderState(5000, AML_DECODER_PLAY))
    AMLAudioPlayer.state = AML_PLAYER_PLAY;
  else {
    PrintErr("Something is wrong, DecoderState:%s\n",
             AMLDecoderStateStr(AMLAudioPlayer.decoder_state));
    ret = AML_ERR_PLAYER_STATE;
  }
  return ret;
}

int AMLAudioPlayer_Pause(void) {
  int ret = 0;

  if (AMLAudioPlayer.state != AML_PLAYER_PLAY) {
    PrintErr("Invalid player state: %s\n",
             AMLPlayerStateStr(AMLAudioPlayer.state));
    return AML_ERR_PLAYER_STATE;
  }

  if (AMLAudioPlayer.decoder_state != AML_DECODER_PLAY) {
    printf("Invalid decoder state: %s\n",
           AMLDecoderStateStr(AMLAudioPlayer.decoder_state));
    return AML_ERR_PLAYER_STATE;
  }

  AMLAudioPlayer.DecCmd = AML_DEC_PAUSE;
  pthread_cond_signal(&AMLAudioPlayer.decoder_cond);

  if (AMLAudioPlayer_WaitForDecoderState(5000, AML_DECODER_PAUSE))
    AMLAudioPlayer.state = AML_PLAYER_PAUSE;
  else {
    PrintErr("Something is wrong, DecoderState:%s\n",
             AMLDecoderStateStr(AMLAudioPlayer.decoder_state));
    ret = AML_ERR_PLAYER_STATE;
  }
  return ret;
}

int AMLAudioPlayer_Resume(void) {
  int ret;
  if (AMLAudioPlayer.state != AML_PLAYER_PAUSE) {
    PrintErr("Invalid player state:%s\n",
             AMLPlayerStateStr(AMLAudioPlayer.state));
    return AML_ERR_PLAYER_STATE;
  }

  if (AMLAudioPlayer.decoder_state != AML_DECODER_PAUSE) {
    printf("Invalid decoder statue:%s\n",
           AMLDecoderStateStr(AMLAudioPlayer.decoder_state));
    return AML_ERR_PLAYER_STATE;
  }

  AMLAudioPlayer.DecCmd = AML_DEC_RESUME;
  pthread_cond_signal(&AMLAudioPlayer.decoder_cond);

  if (AMLAudioPlayer_WaitForDecoderState(5000, AML_DECODER_PLAY))
    AMLAudioPlayer.state = AML_PLAYER_PLAY;
  else {
    PrintErr("Something is wrong, DecoderState:%s\n",
             AMLDecoderStateStr(AMLAudioPlayer.decoder_state));
    ret = AML_ERR_PLAYER_STATE;
  }
  return ret;
}

int AMLAudioPlayer_Stop(void) {
  switch (AMLAudioPlayer.state) {
  case AML_PLAYER_OPEN:
  case AML_PLAYER_PLAY:
  case AML_PLAYER_PAUSE:
    break;
  default:
    PrintErr("Invalid player state:%s\n",
             AMLPlayerStateStr(AMLAudioPlayer.state));
    return AML_ERR_PLAYER_STATE;
  }

  AMLAudioPlayer.DecCmd = AML_DEC_STOP;
  pthread_cond_signal(&AMLAudioPlayer.decoder_cond);

  if (!AMLAudioPlayer_WaitForDecoderState(5000, AML_DECODER_STOP)) {
    PrintErr("Something is wrong, DecoderState:%s\n",
             AMLDecoderStateStr(AMLAudioPlayer.decoder_state));
  }

  // Clear playback URL
  AMLAudioPlayer.URL[0] = '\0';

  if (AMLAudioPlayer.pCodecCtx) {
    avcodec_free_context(&AMLAudioPlayer.pCodecCtx);
    AMLAudioPlayer.pCodecCtx = NULL;
  }
  if (AMLAudioPlayer.in_fmt_ctx) {
    avformat_free_context(AMLAudioPlayer.in_fmt_ctx);
    //    avformat_close_input(AMLAudioPlayer.in_fmt_ctx);
    AMLAudioPlayer.in_fmt_ctx = NULL;
  }
  if (AMLAudioPlayer.out_fmt_ctx) {
    // AMLAudioPlayer.out_stream is no need to close, avformat_free_context will
    // cover it.
#if 0
    if (AMLAudioPlayer.out_stream) {
      ff_free_stream(AMLAudioPlayer.out_fmt_ctx, AMLAudioPlayer.out_stream);
      AMLAudioPlayer.out_stream= NULL;
    }
#endif
    avformat_free_context(AMLAudioPlayer.out_fmt_ctx);
    AMLAudioPlayer.out_fmt_ctx = NULL;
  }
  if (AMLAudioPlayer.out_buffer) {
    av_free(AMLAudioPlayer.out_buffer);
    AMLAudioPlayer.out_buffer = NULL;
  }
  if (AMLAudioPlayer.swr_ctx) {
    swr_free(&AMLAudioPlayer.swr_ctx);
    AMLAudioPlayer.swr_ctx = NULL;
  }
  AMLAudioPlayer.state = AML_PLAYER_CLOSE;

  return 0;
}

#if defined(AML_TEST)
int main(int argc, char **argv) {
  int Quit = 0, ret, InputIndex;
  if (argc < 3) {
    PrintErr("usage: %s alsa_dev input_file [input_url] [input_url]\n",
             argv[0]);
    exit(1);
  }

  ret = AMLAudioPlayer_Init(argv[1], 2, 48000, AV_SAMPLE_FMT_S16);
  SAINT_CHECK(ret == 0);
  InputIndex = 2; //
  while (InputIndex < argc) {
    Quit = 0;
    ret = AMLAudioPlayer_SetInput(argv[InputIndex]);
    SAINT_CHECK(ret == 0);
    InputIndex++;

    ret = AMLAudioPlayer_Play();
    SAINT_CHECK(ret == 0);
    sleep(2);
    printf("s/q for stop and quit, p for pause, r for resume\n");
    while (!Quit) {
      int cmd = getchar();
      switch (cmd) {
      case 'p':
        printf("User want to Pause\n");
        AMLAudioPlayer_Pause();
        break;
      case 'r':
        printf("User want to Resume\n");
        AMLAudioPlayer_Resume();
        break;
      case 's':
      case 'q':
        printf("User want to Stop\n");
        AMLAudioPlayer_Stop();
        Quit = 1;
        break;
      case 10:
        break;
      default:
        printf("Unknown command\n");
        printf("s/q for stop and quit, p for pause, r for resume\n");
        break;
      }
    }
  }
  AMLAudioPlayer_DeInit();
  return 0;
}

#endif
