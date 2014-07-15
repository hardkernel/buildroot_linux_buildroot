# This confidential and proprietary software may be used only as
# authorised by a licensing agreement from ARM Limited
#   (C) COPYRIGHT 2010 - 2011 ARM Limited
#       ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorised
# copies and copies may only be made to the extent permitted
# by a licensing agreement from ARM Limited.
#
# CMakeLists.txt

set (ENV_TOOLCHAIN_ROOT $ENV{TOOLCHAIN_ROOT})

# If the TOOLCHAIN_ROOT environment variable is set use it otherwise rely on the define
if (ENV_TOOLCHAIN_ROOT)
	message ("Using the environment variable TOOLCHAIN_ROOT")
	message ("If you don't want to use the environment variable you must clear it")
	set (TOOLCHAIN_ROOT ${ENV_TOOLCHAIN_ROOT})
else()
	message ("Using the defined variable TOOLCHAIN_ROOT")
endif()

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR arm)

set (CMAKE_CXX_COMPILER "${TOOLCHAIN_ROOT}arm-none-linux-gnueabi-g++")
set (CMAKE_C_COMPILER "${TOOLCHAIN_ROOT}arm-none-linux-gnueabi-gcc")

set (CMAKE_FIND_ROOT_PATH 
    ${TOOLCHAIN_ROOT}
)
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
