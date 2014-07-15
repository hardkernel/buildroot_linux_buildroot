#
# This confidential and proprietary software may be used only as
# authorised by a licensing agreement from ARM Limited
#   (C) COPYRIGHT 2011 ARM Limited
#       ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorised
# copies and copies may only be made to the extent permitted
# by a licensing agreement from ARM Limited.
#

cmake_minimum_required (VERSION 2.8.0)

project(Mali-SDK)

# Don't build an RPATH into the libraries
set(CMAKE_SKIP_RPATH TRUE)

# Decide which target are we building for
if (TARGET)
	message ("Building for ${TARGET}")
else(TARGET)
	if(WIN32)
		set (TARGET "WIN32")
	else(WIN32)
		set (TARGET "LINUX")
	endif(WIN32)
	message ("TARGET was not set. Now using [${TARGET}] as default.")
endif(TARGET)

# Add the DEBUG definition if not building in release mode
if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DDEBUG)
endif()

# Add the Khronos header files and the simple-framework as a shared include location
include_directories(inc)
include_directories(simple-framework/inc)

if(${TARGET} STREQUAL "WIN32")
	# Win32 settings
	set(GLES_LIB "libGLESv2.lib")
	set(EGL_LIB "libEGL.lib")
    set(LIB_DIR "lib/OpenGL-ES-3.0/x86-win32")
    add_definitions(-DWIN32)
    add_definitions(-D_WIN32)
    
    find_library(OPENGLES_LIBRARY_GLES NAMES ${GLES_LIB} PATHS ${LIB_DIR})
    find_library(OPENGLES_LIBRARY_EGL NAMES ${EGL_LIB} PATHS ${LIB_DIR})
    set (OPENGLES_LIBRARIES ${OPENGLES_LIBRARY_GLES} ${OPENGLES_LIBRARY_EGL})
	
	# Set the install prefix for Win32
    # set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/x86)

    # Disable Visual C++ warnings for unsafe functions (Warning C4996)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	
else()
	# Linux and ARM-linux settings
	set (GLES_LIB "libGLESv2.so")
	set (EGL_LIB "libEGL.so")
    
    if (${TARGET} STREQUAL "ARM")
        # Linux on ARM
        set(DUMMY_MALI_LIBRARIES GLESv2 EGL)
        add_subdirectory(lib/OpenGL-ES-3.0/arm-linux)
        set (OPENGLES_LIBRARIES ${DUMMY_MALI_LIBRARIES})
        include_directories(simple-framework/inc/mali)
		
		# Set the install prefix for ARM
        # set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/arm)
    else()
        # Desktop Linux
        set(LIB_DIR "lib/OpenGL-ES-3.0/x86-linux")
        find_library(OPENGLES_LIBRARY_GLES NAMES ${GLES_LIB} PATHS ${LIB_DIR})
        find_library(OPENGLES_LIBRARY_EGL NAMES ${EGL_LIB} PATHS ${LIB_DIR})
        set (OPENGLES_LIBRARIES ${OPENGLES_LIBRARY_GLES} ${OPENGLES_LIBRARY_EGL})
		
		# Set the install prefix for Linux
        # set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/x86)
	endif ()
endif()

set (OpenGLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)

add_subdirectory(simple-framework)

# Add all example sub-directories, inside samples/linux (except .svn)
file(GLOB EXAMPLE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/samples/OpenGL-ES-2.0/linux/*/
    ${CMAKE_CURRENT_SOURCE_DIR}/samples/OpenGL-ES-3.0/linux/*/
    )
    
foreach(EXAMPLE_DIR ${EXAMPLE_DIRS})
    add_subdirectory(${EXAMPLE_DIR})
endforeach()
