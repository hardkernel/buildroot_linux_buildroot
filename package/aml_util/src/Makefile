UTILS = wifi_power 
.PHONY: all install clean

wifi_power: wifi_power.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: $(UTILS)


clean:
	rm -f *.o $(UTILS)

install:
	install -m 755 $(UTILS) $(TARGET_DIR)/usr/bin/

uninstall:
	rm -f $(TARGET_DIR)/usr/bin/
