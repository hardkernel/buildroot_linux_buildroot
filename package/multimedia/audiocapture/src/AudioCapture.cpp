// Copyright 2013 Yahoo! -- All rights reserved.
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "yctv_hacs.h"
#include "AudioCapture.h"
#include "amAudio.h"

// Time (in seconds) to run audio capture in this sample.
#define STOP_AFTER 60

// Print macro
#define LOG printf

// Main loop control flag
bool AudioCaptureOn = true;

void audioCaptureNewData(unsigned char *dataBuff, unsigned dataLength)
{
    iHACS_STAT stat;

    // Check validity of incoming buffer
    if (dataLength && dataBuff)
    {
        if (YCTVACPushData((const char *)dataBuff, dataLength, &stat)==false)
        {
            LOG("Sending Audio PCM data to HACS interface failed.../n");
            return;
        }
    }
    else
    {
        LOG("Callback data is invalid!/n");
        return;
    }

    // Optional status report
    switch(stat)
    {
        case iHACS_STAT_BUSY:
            LOG("HACS interface is BUSY processing, ignoring call...");
            sleep(IHACS_DEF_SEC);
            break;

        case iHACS_STAT_NOT_READY:
            LOG("HACS interface is NOT ready to process data yet...");
            sleep(IHACS_DEF_SEC);
            break;

        case iHACS_STAT_OK:
            LOG("Sent correctly %d bytes...", dataLength);
            break;

        case iHACS_STAT_ERR:
        default:
            LOG("Unknown status %d...", (int)stat);
            break;
    }

    return;
}

// Thread used to stop the audio capture after certain time
void *endCapture(void *data)
{
    // Let timer thread run
    sleep(STOP_AFTER);

    // Stop capture
    AudioCaptureOn = false;
    amAudio_Finish();
    return 0;
}

// AudioCaptureSample entry point
int main(int argn, char **argv)
{
    iHACS_GEOMETRY geo;
    pthread_t timerThread;

    LOG("HACS Interface Version: %s\n", YCTVACGetVersion());

    if (argn>1){
	test_capture(argv[1]);
	return 0;
    }
    // Fill audio geometry structure
    geo.iChannels    = GEOM_CH_STEREO;
    geo.iSampleRate  = GEOM_SR_44KHz;
    geo.iSampleDepth = GEOM_SD_S16;
    geo.iSampleType  = GEOM_FORMAT_SHORT;
    geo.iSwapped     = GEOM_DT_HOST;

    // Configure geometry by calling Settings
    if( YCTVACSettings( geo, SRC_OTHER) == true )
    {
	amAudio_Init(&audioCaptureNewData);
        // Start the timer thread
        pthread_create(&timerThread, NULL, endCapture, NULL);
        pthread_join(timerThread, NULL);
    }
    else
    {
        LOG("IHACS interface could not be configured, exiting...\n");
    }

    // Shutdown HACS interface!
    YCTVACShutdown();

    return 0;
}
