override CFLAGS := -DARM
override CFLAGS += -O2 -fPIC
override LDFLAGS := -lm -lrt -shared
INC = -I./include -I./enc/gx_enc_fast -I.
TARGET = libvpcodec.so

$(TARGET):libvpcodec.o AML_HWEncoder.o enc_api.o gxvenclib_fast.o rate_control_gx_fast.o parser.o
	$(CC) $(LDFLAGS) -o $@ $^

libvpcodec.o: libvpcodec.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

AML_HWEncoder.o: AML_HWEncoder.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

enc_api.o: enc_api.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

gxvenclib_fast.o: ./enc/gx_enc_fast/gxvenclib_fast.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

rate_control_gx_fast.o: ./enc/gx_enc_fast/rate_control_gx_fast.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

parser.o: ./enc/gx_enc_fast/parser.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
