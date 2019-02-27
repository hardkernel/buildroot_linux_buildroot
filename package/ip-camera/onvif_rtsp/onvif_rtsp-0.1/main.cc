#include "onvif_rtsp_server.h"
#include "onvif_rtsp_storage.h"
#include "onvif_rtsp_source.h"

static RTSP_SERVER_t server = {0};

static void
sig_handler (int signo) {
  switch (signo) {
    case SIGINT:
      if (server.loop) {
        g_main_loop_quit (server.loop);
      }
      break;
  }
}

int main (int argc, char **argv) {
  std::shared_ptr<CONFIG_t> config = std::make_shared<CONFIG_t> ();
  server.config = config;

  signal (SIGINT, sig_handler);

  rtsp_config_init (config);

  gst_init (NULL, NULL);

  server.loop = g_main_loop_new (NULL, FALSE);

  if (!rtsp_source_start (&server)) {
    g_print ("Failed to start source pipeline\n");
    return -1;
  }

  if (!rtsp_storage_start (&server)) {
    g_print ("Failed to start storage pipeline\n");
    return -1;
  }

  if (!rtsp_server_init (&server)) {
    g_print ("Failed to init rtsp server\n");
    return -1;
  }

  if (!rtsp_server_start (&server)) {
    g_print ("Failed to start rtsp server\n");
    return -1;
  }


  g_main_loop_run (server.loop);

  rtsp_storage_stop (&server);
  rtsp_source_stop (&server);

  return 0;
}
