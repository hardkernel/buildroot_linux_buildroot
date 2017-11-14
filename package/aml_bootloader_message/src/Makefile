LIB = libbootloader_message.a
.PHONY: all install clean

bootloader_message.o: bootloader_message.c
	$(CC) -c bootloader_message.c

all: bootloader_message.o
	$(AR) rc $(LIB) bootloader_message.o

clean:
	rm -f *.o $(LIB)

install:
	install -m 755 $(LIB) $(STAGING_DIR)/usr/lib

