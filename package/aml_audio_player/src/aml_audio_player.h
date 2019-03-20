#ifndef __AML_AUDIO_PLAYER__
#define __AML_AUDIO_PLAYER__

enum {
  AML_NO_ERR = 0,
  AML_ERR_UNKNOWN = -1,        /*Unknown error*/
  AML_ERR_INVALID_INPUT = -2,  /*Can't open input */
  AML_ERR_INPUT_NO_AUDIO = -3, /*There is no audio from the input stream*/
  AML_ERR_SYSTEM = -4,         /*System error*/
  AML_ERR_PLAYER_INT = -5,     /*Player internal error*/
  AML_ERR_PLAYER_STATE = -6,   /*Player State error*/
  AML_ERR_OUTPUT = -7,         /*Ouput error*/
} AMLAudioPlayerErrCode_e;

enum {
  AML_AUDIO_PLAYER_CB_EOF = 0,
} AMLAudioPlayerCB_e;

typedef void (*AMLAudioPlayerCallback)(int type, void *param);

int AMLAudioPlayer_Init(char *OutDev, int out_channels, int out_samplerate,
                        int out_format);
int AMLAudioPlayer_DeInit(void);
int AMLAudioPlayer_SetInput(char *URL);
int AMLAudioPlayer_Play(void);
int AMLAudioPlayer_Pause(void);
int AMLAudioPlayer_Resume(void);
int AMLAudioPlayer_Stop(void);

int AMLAudioPlayer_Install_CB(AMLAudioPlayerCallback callback);

#endif
