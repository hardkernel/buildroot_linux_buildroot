UTILS = dsp_util 
.PHONY: all install clean

$(info CFLAGS = $(CFLAGS))
dsp_util: dsp_util.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: $(UTILS)

clean:
	rm -f *.o $(UTILS)

install: $(UTILS)
	install -m 755 $(UTILS) $(TARGET_DIR)/usr/bin/

uninstall:
	rm -f $(TARGET_DIR)/usr/bin/$(UTILS)
