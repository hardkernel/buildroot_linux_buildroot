override CFLAGS := -DARM
override CFLAGS += -g -fPIC
override LDFLAGS := -lm -lrt -shared
INC = -I./include -I./libge2d -I.
TARGET = libvphevcodec.so

C_SRCS := $(wildcard *.c) $(wildcard **/*.c)
C_OBJS := $(C_SRCS:%.c=%.o)

CXX_SRCS := $(wildcard *.cpp) $(wildcard **/*.cpp)
CXX_OBJS := $(CXX_SRCS:%.cpp=%.o)

$(TARGET): $(C_OBJS) $(CXX_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INC) $< -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
