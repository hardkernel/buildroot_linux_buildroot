#include <libavutil/samplefmt.h>
#include <unistd.h>
#include "aml_audio_player.h"

#define PrintErr(...) {fprintf(stderr, "File:%s Line:%d, Func:%s ==> ", __FILE__, __LINE__, __func__); fprintf(stderr,  __VA_ARGS__);}
#define SAINT_CHECK(X) if(!(X)) { PrintErr("Assert (%s)\n", #X); exit(1);}

int main (int argc, char **argv)
{
    int Quit=0,ret, InputIndex;
    if (argc < 3) {
        PrintErr("usage: %s alsa_dev input_file [input_url] [input_url]\n", argv[0]);
        exit(1);
    }

    ret = AMLAudioPlayer_Init(argv[1], 2 , 48000, AV_SAMPLE_FMT_S16);
    SAINT_CHECK(ret == 0);
    InputIndex=2;   //
    while (InputIndex<argc)
    {
        ret = AMLAudioPlayer_SetInput(argv[InputIndex]);
        SAINT_CHECK(ret == 0);
        InputIndex ++;

        ret = AMLAudioPlayer_Play();
        SAINT_CHECK(ret == 0);
        sleep(1);
        printf("s/q for stop and quit, p for pause, r for resume\n");
        Quit = 0;
        while (!Quit)
        {
            int cmd=getchar();
            switch (cmd)
            {
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

