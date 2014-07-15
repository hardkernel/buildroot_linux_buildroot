#
# This confidential and proprietary software may be used only as
# authorised by a licensing agreement from ARM Limited
#   (C) COPYRIGHT 2012 ARM Limited
#       ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorised
# copies and copies may only be made to the extent permitted
# by a licensing agreement from ARM Limited.
#

cmake_minimum_required (VERSION 2.8)

project (simple-framework)

file(GLOB COMMMON_SOURCES
    src/EGLRuntime.cpp
    src/ETCHeader.cpp
    src/Text.cpp
    src/Matrix.cpp
    src/Platform.cpp
    src/Shader.cpp
    src/Texture.cpp    
    src/Timer.cpp
)

file(GLOB GLES3_SOURCES
    src/Geometry.cpp
)

if(${TARGET} STREQUAL "WIN32")
    # Win32 settings
    file(GLOB PLATFORM_SOURCES
        src/WindowsPlatform.cpp
    )
endif()
if(${TARGET} STREQUAL "ARM")
    # Linux and ARM-linux settings
    file(GLOB PLATFORM_SOURCES
        src/LinuxOnARMPlatform.cpp
    )
endif()
if(${TARGET} STREQUAL "LINUX")
    # Linux and ARM-linux settings
    file(GLOB PLATFORM_SOURCES
        src/DesktopLinuxPlatform.cpp
    )
endif()

add_library(simple-framework2 ${COMMMON_SOURCES} ${PLATFORM_SOURCES})
add_library(simple-framework3 ${COMMMON_SOURCES} ${GLES3_SOURCES} ${PLATFORM_SOURCES})

if(${TARGET} STREQUAL "LINUX")
    set_target_properties(simple-framework2 PROPERTIES COMPILE_FLAGS "-m32 -DGLES_VERSION=2" LINK_FLAGS "-m32")
    set_target_properties(simple-framework3 PROPERTIES COMPILE_FLAGS "-m32 -DGLES_VERSION=3" LINK_FLAGS "-m32")
else()
    set_target_properties(simple-framework2 PROPERTIES COMPILE_FLAGS "-DGLES_VERSION=2")
    set_target_properties(simple-framework3 PROPERTIES COMPILE_FLAGS "-DGLES_VERSION=3")
endif()
