CPPFLAGS += $(shell $(PKG_CONFIG) --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0)
LDFLAGS += $(shell $(PKG_CONFIG) --libs gstreamer-1.0 gstreamer-rtsp-server-1.0)

OBJS := $(patsubst %cc,%o,$(wildcard *.cc))

onvif_rtsp : $(OBJS)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) $^ -o $@
