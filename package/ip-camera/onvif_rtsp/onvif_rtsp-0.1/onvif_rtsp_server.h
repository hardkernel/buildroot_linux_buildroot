#ifndef _ONVIF_RTSP_SERVER_
#define _ONVIF_RTSP_SERVER_

#include <gst/gst.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <gst/rtsp-server/rtsp-onvif-server.h>
#ifdef __cplusplus
}
#endif
#include "onvif_rtsp_config.h"
#include "onvif_rtsp_pipeline.h"

typedef struct rtsp_server {
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;

  RTSP_PIPELINE_t pipelines;

  GstRTSPAuth *auth;
  GstRTSPToken *token;
  gchar *basic;

  std::shared_ptr<CONFIG_t> config;
} RTSP_SERVER_t;

bool rtsp_server_init(RTSP_SERVER_t *serv);
bool rtsp_server_start(RTSP_SERVER_t *serv);

#endif /* _ONVIF_RTSP_SERVER_ */
