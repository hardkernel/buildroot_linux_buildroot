CPPFLAGS += $(shell $(PKG_CONFIG) --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0)
LDFLAGS += $(shell $(PKG_CONFIG) --libs gstreamer-1.0 gstreamer-rtsp-server-1.0)

onvif_rtsp : onvif_rtsp_config.o onvif_rtsp_server.o main.o
	$(CXX) $(CPPFLAGS) $(LDFLAGS) $^ -o $@
