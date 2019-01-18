#include "onvif_rtsp_server.h"

int main(int argc, char **argv) {
  RTSP_SERVER_t server;
  std::shared_ptr<RTSP_CONFIG_t> config = std::make_shared<RTSP_CONFIG_t>();
  server.config = config;

  rtsp_config_init(config);

  gst_init(NULL, NULL);

  if (!rtsp_server_init (&server)) {
    g_print ("Failed to init server\n");
    return -1;
  }

  return rtsp_server_start(&server);
}
