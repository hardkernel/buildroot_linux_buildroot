#include "onvif_rtsp_server.h"

static
std::string create_pipeline(std::shared_ptr<RTSP_CONFIG_t> &config) {
  if (not config->debug.pipeline.empty()) {
    return config->debug.pipeline;
  }
  std::string pipeline = "( ";
  pipeline += "v4l2src device=";
  pipeline += config->video.device;

  pipeline += " ! videoscale ! video/x-raw,width=";
  pipeline += config->video.scale.first;
  pipeline += ",height=";
  pipeline += config->video.scale.second;

  if (config->face_detection) {
    pipeline += " ! amlyoloface";
  }

  pipeline += " ! amloverlay";
  if (not config->overlay_options.empty()) {
    pipeline += " ";
    pipeline += config->overlay_options;
  }

  pipeline += " ! amlvenc framerate=";
  pipeline += config->video.framerate;

  pipeline += " bitrate=";
  pipeline += config->video.bitrate;

  pipeline += " gop=";
  pipeline += config->video.gop;

  if (config->video.use_x265) {
    pipeline += " ! rtph265pay";
  } else {
    pipeline += " ! rtph264pay";
  }
  pipeline += " name=pay0 pt=96";

  if (config->audio.device == "test") {
    pipeline += " ! audiotestsrc is-live=true wave=white-noise";
  } else {
    pipeline += " ! alsasrc device=";
    pipeline += config->audio.device;
  }

  if (config->audio.codec == "mulaw") {
    pipeline += " ! mulawenc ! rtppcmupay name=pay1";
  }

  pipeline += " )";
  return pipeline;
}

static
std::string create_backchanel_pipeline(std::shared_ptr<RTSP_CONFIG_t> &config) {
  if (not config->debug.backchannel.empty()) {
    return config->debug.backchannel;
  }
  std::string pipeline = " capsfilter caps=\"application/x-rtp,media=audio,payload=0,clock-rate=";
  pipeline += config->audio.backchannel.clock_rate;
  pipeline += ",encoding-name=";
  pipeline += config->audio.backchannel.encoding;
  pipeline += "\" name=depay_backchannel !";
  if (config->audio.backchannel.encoding == "PCMU") {
    pipeline += " rtppcmudepay";
  }
  pipeline += " ! fakesink async=false";

  return pipeline;
}


bool rtsp_server_init(RTSP_SERVER_t *serv) {
  serv->loop = g_main_loop_new (NULL, FALSE);
  /* create a server instance */
  serv->server = gst_rtsp_onvif_server_new ();

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  serv->mounts = gst_rtsp_server_get_mount_points (serv->server);

  /*  Set the address and port to bind */
  gst_rtsp_server_set_address(serv->server, serv->config->network.address.c_str());
  gst_rtsp_server_set_service(serv->server, serv->config->network.port.c_str());

  auto &&session = gst_rtsp_session_new("New session");
  gst_rtsp_session_prevent_expire(session);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  serv->factory = gst_rtsp_onvif_media_factory_new ();

  gst_rtsp_media_factory_set_shared (serv->factory, TRUE);
  gst_rtsp_media_factory_set_media_gtype (serv->factory, GST_TYPE_RTSP_ONVIF_MEDIA);
  gst_rtsp_media_factory_set_latency(serv->factory, 100);
  gst_rtsp_media_factory_set_publish_clock_mode(serv->factory, GST_RTSP_PUBLISH_CLOCK_MODE_CLOCK_AND_OFFSET);

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory (serv->mounts,
      serv->config->network.route.c_str(), serv->factory);

  /* don't need the ref to the mapper anymore */
  g_object_unref (serv->mounts);

  if (not serv->config->auth.username.empty()) {
	/* the user can look at the media but not construct so he gets a
	 * 401 Unauthorized */
	gst_rtsp_media_factory_add_role(
		serv->factory, serv->config->auth.username.c_str(),
		GST_RTSP_PERM_MEDIA_FACTORY_ACCESS, G_TYPE_BOOLEAN, true,
		GST_RTSP_PERM_MEDIA_FACTORY_CONSTRUCT, G_TYPE_BOOLEAN, true, NULL);

	/* make a new authentication manager */
	serv->auth = gst_rtsp_auth_new();

	/* make admin token */
	serv->token =
	  gst_rtsp_token_new(GST_RTSP_TOKEN_MEDIA_FACTORY_ROLE, G_TYPE_STRING,
		  serv->config->auth.username.c_str(), NULL);

    serv->basic = gst_rtsp_auth_make_basic(serv->config->auth.username.c_str(),
                                           serv->config->auth.password.c_str());
    gst_rtsp_auth_add_basic(serv->auth, serv->basic, serv->token);
    g_free(serv->basic);
    gst_rtsp_token_unref(serv->token);

    /* set as the server authentication manager */
    gst_rtsp_server_set_auth(serv->server, serv->auth);
    g_object_unref(serv->auth);

  }

  std::string pipeline = create_pipeline(serv->config);
  gst_rtsp_media_factory_set_launch (serv->factory, pipeline.c_str());
  g_print ("Stream pipeline:\n  %s\n", pipeline.c_str());

  pipeline = create_backchanel_pipeline(serv->config);
  gst_rtsp_onvif_media_factory_set_backchannel_launch(GST_RTSP_ONVIF_MEDIA_FACTORY(serv->factory),
      pipeline.c_str());
  g_print ("Backchannel pipeline:\n  %s\n", pipeline.c_str());

  return true;
}

bool rtsp_server_start(RTSP_SERVER_t *serv)
{
  /* attach the server to the default maincontext */
  if (gst_rtsp_server_attach (serv->server, NULL) == 0) {
    g_print("failed to attach the server");
    return false;
  }

  /* start serving */
  g_print ("stream ready at rtsp://%s:%s%s\n", serv->config->network.address.c_str(),
      serv->config->network.port.c_str(), serv->config->network.route.c_str());
  g_main_loop_run (serv->loop);

  return 0;
}
