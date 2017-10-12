LIB = libubootenv.a
.PHONY: all install clean

ubootenv.o: ubootenv.c
	$(CC) -c ubootenv.c

all: ubootenv.o
	$(AR) rc $(LIB) ubootenv.o

clean:
	rm -f *.o $(LIB)

install:
	install -m 755 $(LIB) $(STAGING_DIR)/usr/lib

