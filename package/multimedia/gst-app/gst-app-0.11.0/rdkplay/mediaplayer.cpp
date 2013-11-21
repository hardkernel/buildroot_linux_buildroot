/*
 * ============================================================================
 * COMCAST C O N F I D E N T I A L AND PROPRIETARY
 * ============================================================================
 * This file and its contents are the intellectual property of Comcast.  It may
 * not be used, copied, distributed or otherwise  disclosed in whole or in part
 * without the express written permission of Comcast.
 * ============================================================================
 * Copyright (c) 2013 Comcast. All rights reserved.
 * ============================================================================
 */
#include <stdio.h>
#include <unistd.h>
#include <gst/gst.h>
#include <string.h>
#include <math.h>

GstElement *pipeline, *source, *demuxer, *decoder, *v_sink, *a_conv, *a_proc, *a_sink;
static float play_speed = 0.0;
static int rewind_speed = 0;
static int forward_speed = 0;
static int mediaStop = 0;
static int trickPlay = 0;
static gulong gCurrentPosition = 0;

#define MPLAYER_SOURCE      "filesrc"
#define MPLAYER_DECODER    "decodebin"
#define MPLAYER_V_SINK       "amlvsink" //"sdlvideosink"
#define MPLAYER_A_CONV      "audioconvert"
#define MPLAYER_A_PROC      "audioresample"
#define MPLAYER_A_SINK       "amlasink"            // "autoaudiosink"


static gboolean bus_call(GstBus * bus, GstMessage * msg, gpointer data)
{
    g_print("[MediaPlayer bus_call] msg=%d,%s \n", GST_MESSAGE_TYPE(msg), GST_MESSAGE_TYPE_NAME(msg));

    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_EOS:
            g_print ("End of stream\n");
            break;
        case GST_MESSAGE_ERROR:
        case GST_MESSAGE_WARNING:
        case GST_MESSAGE_APPLICATION:
            {
                gchar *debug;
                GError *error;
                gst_message_parse_error (msg, &error, &debug);
                g_free (debug);
                g_printerr ("Error: %s\n", error->message);
                g_error_free (error);
                break;
            }
        case GST_MESSAGE_STATE_CHANGED:
            g_print ("State Change...\n");
        default: 
            break;
    }

   return TRUE;
}

static void mplayer_position_update()
{
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 pos = 0;

    gst_element_query_position (pipeline, &fmt, &pos);
    gCurrentPosition = GST_TIME_AS_MSECONDS(pos);
    g_print ("Current Position=%lu\n", gCurrentPosition);
    return;
}

void usuage ()
{
    g_print ("\n");
    g_print ("************************************************\n");
    g_print ("*****  Media Player Standalone Application *****\n");
    g_print ("************************************************\n");
    g_print ("  p. Pause\n");
    g_print ("  l. Play\n");
    g_print ("  s. Stop playback\n");
    g_print ("  r. Rewind (incremental)\n");
    g_print ("  f. Forward (incremental)\n");
    g_print ("  h. Get help\n");
    g_print ("  q. Quit\n");
}


int configureAVElement (GstPad* pad, GstElement *element)
{
    int linked = 0;
    GstPad* sinkpad = gst_element_get_static_pad (element, "sink");
    if(!GST_PAD_IS_LINKED(sinkpad))
    {
        linked = GST_PAD_LINK_SUCCESSFUL(gst_pad_link (pad, sinkpad));

        if (!linked)
        {
            g_print ("Gstreamer: Failed to link demux and AV decoders\n");
        }
        else
        {
            g_print("Configured Audio/Video \n");
        }
    }
    else
        g_print("Already linked\n");

    gst_object_unref (sinkpad);

   return linked;
}

static void on_pad_added (GstElement *element, GstPad *pad, gboolean last, gpointer data)
{
    if (!pipeline) return;

    element = element;
    data = data;
    last = last;

    GstCaps *caps = gst_pad_get_caps(pad);
    GstStructure *s = gst_caps_get_structure (caps, 0);

    if (strncmp (gst_structure_get_name(s), "video/", 6) == 0)
    {
        g_print ("Gstreamer: Configure Video decoders\n");
        configureAVElement (pad, v_sink);
        return;
    }

    if (strncmp (gst_structure_get_name(s), "audio/", 6) == 0)
    {
        g_print ("Gstreamer: Configure Audio decoders\n");
        configureAVElement (pad, a_conv);
    }
}

static void on_auto_element_added (GstBin *pDecoderBin, GstElement *pElement, gpointer   data)
{
    g_print("Element added automatically.. It is %s\n", GST_ELEMENT_NAME (pElement));
}

static void load_url (char* pURL)
{
    if (pURL)
    {
        /* Set the input filename to the source element */
        g_print ("url = %s\n", pURL);
        g_object_set (G_OBJECT (source), "location", pURL, NULL);
    }
    else
    {
        g_print ("Invalid URL..\n");
    }
}

// Wrapper Function for gst_element_factory_make for creating an element
static GstElement *gst_element_factory_make_or_warn (const gchar * type, gchar * name)
{
    GstElement *element = gst_element_factory_make (type, name);
    if (!element) {
        g_warning ("Failed to create element %s of type %s", name, type);
    }
    return element;
}

static int load_elements (char* pURL)
{
    guint frame_delay = 0;

    /* Create gstreamer elements */
    pipeline = gst_pipeline_new ("Mplayer-Standalone");

    source  = gst_element_factory_make_or_warn (MPLAYER_SOURCE, NULL);
    decoder = gst_element_factory_make_or_warn (MPLAYER_DECODER, NULL);
    v_sink  = gst_element_factory_make_or_warn (MPLAYER_V_SINK, NULL);
    a_conv  = gst_element_factory_make_or_warn (MPLAYER_A_CONV, NULL);
    a_proc  = gst_element_factory_make_or_warn (MPLAYER_A_PROC, NULL);
    a_sink  = gst_element_factory_make_or_warn (MPLAYER_A_SINK, NULL);

    /* Set the source path to get the stream from */
    load_url (pURL);

    /* Add Elements to the pipelines */
    gst_bin_add_many (GST_BIN(pipeline), source, decoder, v_sink, a_conv, a_proc, a_sink, NULL);

    /* we link the elements together */
    gst_element_link (source, decoder);
    gst_element_link (a_conv, a_proc);
    gst_element_link (a_proc, a_sink);

    //listening for End Of Stream (EOS) events, etc.
    GstBus* bin_bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bin_bus, bus_call, NULL);
    gst_object_unref(bin_bus);

    //will try to connect demux with viddec and audio on the fly
    g_signal_connect (decoder, "new-decoded-pad", G_CALLBACK (on_pad_added), NULL);
    g_signal_connect (decoder, "element-added", G_CALLBACK (on_auto_element_added),  NULL);
    return 0;
}

static void mplayer_play (void)
{
    if (!pipeline) return;

    if (GST_STATE_CHANGE_FAILURE == gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING))
    {
        g_print ("Failed to PLAY\n");
        return;
    }
}

static void mplayer_ready (void)
{
    if (!pipeline) return;

    mplayer_position_update();

    if (GST_STATE_CHANGE_FAILURE == gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_READY))
    {
        g_print ("Failed to PAUSE\n");
        return;
    }
}

static void mplayer_pause (void)
{
    if (!pipeline) return;

    mplayer_position_update();

    if (GST_STATE_CHANGE_FAILURE == gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED))
    {
        g_print ("Failed to PAUSE\n");
        return;
    }

}

static void mplayer_stop()
{

    if (!pipeline) return;

    mplayer_position_update();

    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);

    gst_object_unref(pipeline);

    pipeline = NULL;
    source = NULL;
    demuxer = NULL;
    decoder = NULL;
    v_sink = NULL;
    a_conv = NULL;
    a_proc = NULL;
    a_sink = NULL;
    g_print ("STOPPED\n");
}

char*  update_rewind(char* pURL)
{
    static char urlStr[1024] = "";

    if (rewind_speed < 4)
    {
        trickPlay = 1;
        rewind_speed++;
        if (rewind_speed == 1)
            play_speed=-4.000000;
        else if (rewind_speed == 2)
            play_speed=-15.000000;
        else if (rewind_speed == 3)
            play_speed=-30.000000;
        else if (rewind_speed == 4)
            play_speed=-60.000000;

    }
    else
    {
        rewind_speed = 0;
        trickPlay = 0;
        play_speed = 1.0;
        sprintf (urlStr, "%s", pURL);
    }

    sprintf (urlStr, "%s&play_speed=%f&time_pos=%lu", pURL, play_speed, gCurrentPosition);
    return urlStr;
}

void mplayer_trickplay(char* pURL)
{
    /* Load the URL */
    load_url (pURL);

    /* Seek the pipeline to avoid the video freeze/jerky */
    if (play_speed >= 0)
        gst_element_seek (GST_ELEMENT(pipeline), play_speed, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, gCurrentPosition * GST_MSECOND, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

    /* Mute & Unmute the audio */
    if ((play_speed > 2) || (play_speed < 0))
        g_object_set (G_OBJECT (a_sink), "mute", TRUE, NULL);
    else
        g_object_set (G_OBJECT (a_sink), "mute", FALSE, NULL);
}

char*  update_forward(char* pURL)
{
    static char urlStr[1024] = "";

    if (forward_speed < 4)
    {
        trickPlay = 1;
        forward_speed++;
        if (forward_speed == 1)
            play_speed=4.000000;
        else if (forward_speed == 2)
            play_speed=15.000000;
        else if (forward_speed == 3)
            play_speed=30.000000;
        else if (forward_speed == 4)
            play_speed=60.000000;
    }
    else
    {
        trickPlay = 0;
        forward_speed = 0;
        play_speed = 1.0;
    }

    sprintf (urlStr, "%s&play_speed=%f&time_pos=%lu", pURL, play_speed, gCurrentPosition);
    return urlStr;
}


int main (int argc, char *argv[])
{
    char *pCurrentURL = NULL;
    char *pPlayURL1 = NULL;
    char *pPlayURL2 = NULL;
    int loopFlag = 1;
    int inputCommand = 0;
    char arr[1024] = "";
    /* Check input arguments */
    if (argc < 2)
    {
        g_printerr ("Usage: %s <url to play>\n", argv[0]);
        return -1;
    }
    
    pPlayURL1 = argv[1];
    pPlayURL2 = argv[2];

    /* Initialisation */
    gst_init (&argc, &argv);


    load_elements (pPlayURL1);
    pCurrentURL = pPlayURL1;
    mplayer_play();
    
    while (loopFlag)
    {
        usuage();
        g_print ("Enter your command here#\n");
        inputCommand = getchar ();
        getchar ();
        fflush(NULL);
        switch (inputCommand)
        {
                case 'l':
                        rewind_speed = 0;
                        forward_speed = 0;
                        play_speed = 0.0;

                        /* When FW is active and wants to play, then we have to update
                           the speed as 1 and just play from at that position */
                        if (trickPlay == 1)
                        {
                            trickPlay = 0;

                            /* Set to Max value; the update_forward will reset to 0 */
                            forward_speed = 255;

                            mplayer_ready();
                            mplayer_trickplay(update_forward(pCurrentURL));
                        }
                        else if (mediaStop == 1)
                        {
                            mediaStop = 0;
                            mplayer_trickplay(pCurrentURL);
                        }
                        
                        mplayer_play();
                        break;
                case 'p':
                        mplayer_pause();
                        break;
                case 's':
                        mediaStop = 1;
                        mplayer_ready();
                        break;
                case 'q':
                        mplayer_stop();
                        loopFlag = 0;
                        break;
                case 'r':
                        mplayer_pause();
                        mplayer_trickplay(update_rewind (pCurrentURL));
                        mplayer_play();
                        break;
                case 'f':
                        mplayer_pause();
                        mplayer_trickplay(update_forward(pCurrentURL));
                        mplayer_play();
                default:
                        break;
        }
    }

    return 0;
}

