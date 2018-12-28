include $(AQROOT)/makefile.linux.def


INCLUDE += -I$(SDK_DIR)/include -I$(SDK_DIR)/include/HAL -I$(AQROOT)/sdk/inc  -I./
INCLUDE += -I$(DFB_DIR)
CFLAGS += $(INCLUDE) -fPIC

################################################################################
# Supply necessary libraries.
LIBS += -L$(AQROOT)/sharelib -l OpenVX -l OpenVXU -l CLC -l VSC -l m -lGAL -l ovxlib -l LLVM_viv
LIBS += -lm -ldl -ldirectfb -pthread

#############################################################################
# Macros.
PROGRAM = 1
DYNAMIC = 1
TARGET_NAME = libyoloface.so
CUR_SOURCE = ${wildcard *.c}
#############################################################################
# Objects.
OBJECTS =  ${patsubst %.c, $(OBJ_DIR)/%.o, $(CUR_SOURCE)}
# installation directory
INSTALL_DIR := ./

################################################################################
# Include the common makefile.

include $(AQROOT)/common.target
