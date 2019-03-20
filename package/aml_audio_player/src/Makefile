target= aml_audio_player libaml_audio_player.so

CFLAGS += -Wall -g -DAML_DEBUG -fPIC

SRC=  aml_audio_player aml_audio_player_test

OBJS=$(addsuffix .o,$(SRC))

all: $(OBJS) $(target)

libaml_audio_player.so: aml_audio_player.o
	$(CC) aml_audio_player.o  -lavdevice -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -lpthread -shared -o $@

aml_audio_player: libaml_audio_player.so aml_audio_player_test.o
	$(CC) aml_audio_player_test.o  -L./ -laml_audio_player -o $@

clean:
	rm $(target)

install:
	install -m 0644 libaml_audio_player.so $(TARGET_DIR)/usr/lib/
	install -m 0755 aml_audio_player $(TARGET_DIR)/usr/bin/
	install -m 0644 aml_audio_player.h $(STAGING_DIR)/usr/include/
	install -m 0644 libaml_audio_player.so $(STAGING_DIR)/usr/lib/

uninstall:
	rm -f $(TARGET_DIR)/usr/bin/aml_audio_player
	rm -f $(TARGET_DIR)/usr/lib/libaml_audio_player.so

