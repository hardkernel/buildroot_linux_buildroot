# Makefile for TRUSTZONE

TARGET_ASMFLAGS = -DCONFIG_CORTEX_A9=1 -mcpu=cortex-a9
EXTRA_CFLAGS = -g -Wall -nostdlib
EXTRA_CFLAGS += -msoft-float -mno-thumb-interwork -marm -fno-short-enums $(TARGET_CCFLAGS)

TRUSTZONE_LIB = y
ifneq ($(KBUILD_SRC),)
TOP_KBUILD_SRC := $(KBUILD_SRC)/
endif

ifeq ($(TRUSTZONE_LIB), y)
$(obj)/otz_client_main :
	@cp $(TOP_KBUILD_SRC)drivers/amlogic/trustzone/otz_client_main.o_shipped \
    drivers/amlogic/trustzone/otz_client_main.o
$(obj)/otz_client_main.o : $(obj)/otz_client_main FORCE
else
ccflags-y:= -I$(TOP_KBUILD_SRC)drivers/amlogic/trustzone/include
endif

otz_client-objs += otz_client_main.o
obj-$(CONFIG_MESON_TRUSTZONE) += otz_client.o
