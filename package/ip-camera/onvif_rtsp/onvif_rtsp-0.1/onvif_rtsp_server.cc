#include "onvif_rtsp_server.h"

static void
on_client_media_configure (GstRTSPMediaFactory *factory, GstRTSPMedia *media, RTSP_SERVER_t *serv) {
  PIPELINE_SRC_t *src = &serv->pipelines.src;
  std::shared_ptr<CONFIG_t> config = serv->config;

  GstElement *rtp = gst_rtsp_media_get_element (media);

  GstElement *vsrc = gst_bin_get_by_name (GST_BIN (rtp), "vsrc");
  if (src->vsink_caps) {
    g_object_set (G_OBJECT (vsrc),
        "caps", src->vsink_caps,
        NULL);
    gst_object_unref (vsrc);
  }

  if (!config->debug.disable_audio) {
    GstElement *asrc = gst_bin_get_by_name (GST_BIN (rtp), "asrc");
    if (src->asink_caps) {
      g_object_set (G_OBJECT (asrc),
          "caps", src->asink_caps,
          NULL);
    }
    gst_object_unref (asrc);
  }

  gst_object_unref (rtp);
}

bool rtsp_server_init (RTSP_SERVER_t *serv) {
  /* create a server instance */
  serv->server = gst_rtsp_onvif_server_new ();

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  serv->mounts = gst_rtsp_server_get_mount_points (serv->server);

  /*  Set the address and port to bind */
  gst_rtsp_server_set_address (serv->server, serv->config->network.address.c_str ());
  gst_rtsp_server_set_service (serv->server, serv->config->network.port.c_str ());

  auto &&session = gst_rtsp_session_new ("New session");
  gst_rtsp_session_prevent_expire (session);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  serv->factory = gst_rtsp_onvif_media_factory_new ();

  g_signal_connect (serv->factory, "media-configure", G_CALLBACK (on_client_media_configure), serv);

  gst_rtsp_media_factory_set_shared (serv->factory, TRUE);
  gst_rtsp_media_factory_set_media_gtype (serv->factory, GST_TYPE_RTSP_ONVIF_MEDIA);
  gst_rtsp_media_factory_set_latency (serv->factory, 100);
  gst_rtsp_media_factory_set_publish_clock_mode (serv->factory, GST_RTSP_PUBLISH_CLOCK_MODE_CLOCK_AND_OFFSET);

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory (serv->mounts,
      serv->config->network.route.c_str (), serv->factory);

  /* don't need the ref to the mapper anymore */
  g_object_unref (serv->mounts);

  if (not serv->config->auth.username.empty ()) {
    /* the user can look at the media but not construct so he gets a
     * 401 Unauthorized */
    gst_rtsp_media_factory_add_role (
        serv->factory, serv->config->auth.username.c_str (),
        GST_RTSP_PERM_MEDIA_FACTORY_ACCESS, G_TYPE_BOOLEAN, true,
        GST_RTSP_PERM_MEDIA_FACTORY_CONSTRUCT, G_TYPE_BOOLEAN, true, NULL);

    /* make a new authentication manager */
    serv->auth = gst_rtsp_auth_new ();

    /* make admin token */
    serv->token =
      gst_rtsp_token_new (GST_RTSP_TOKEN_MEDIA_FACTORY_ROLE, G_TYPE_STRING,
          serv->config->auth.username.c_str (), NULL);

    serv->basic = gst_rtsp_auth_make_basic (serv->config->auth.username.c_str (),
                                           serv->config->auth.password.c_str ());
    gst_rtsp_auth_add_basic (serv->auth, serv->basic, serv->token);
    g_free (serv->basic);
    gst_rtsp_token_unref (serv->token);

    /* set as the server authentication manager */
    gst_rtsp_server_set_auth (serv->server, serv->auth);
    g_object_unref (serv->auth);

  }


  std::string pipeline = pipeline_create_rtp (serv->config);
  gst_rtsp_media_factory_set_launch (serv->factory, pipeline.c_str ());

  if (!serv->config->debug.disable_backchannel) {
    pipeline = pipeline_create_backchannel (serv->config);
    gst_rtsp_onvif_media_factory_set_backchannel_launch (GST_RTSP_ONVIF_MEDIA_FACTORY (serv->factory),
        pipeline.c_str ());
  }

  return true;
}

bool rtsp_server_start (RTSP_SERVER_t *serv)
{

  /* attach the server to the default maincontext */
  if (gst_rtsp_server_attach (serv->server, NULL) == 0) {
    g_print ("failed to attach the server");
    return false;
  }

  /* start serving */
  g_print ("stream ready at rtsp://%s:%s%s\n", serv->config->network.address.c_str (),
      serv->config->network.port.c_str (), serv->config->network.route.c_str ());

  return true;
}
