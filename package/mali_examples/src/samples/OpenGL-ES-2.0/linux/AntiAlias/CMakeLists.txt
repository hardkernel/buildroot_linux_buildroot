#
# This confidential and proprietary software may be used only as
# authorised by a licensing agreement from ARM Limited
#   (C) COPYRIGHT 2012 ARM Limited
#       ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorised
# copies and copies may only be made to the extent permitted
# by a licensing agreement from ARM Limited.
#

# Mali sample: AntiAlias

project (AntiAlias)

set(EXECUTABLE_NAME ${PROJECT_NAME})

# Retrieve all the source file names inside the current folder.
file(GLOB EXAMPLE_SOURCES *.cpp)

# Create an executable file called AntiAlias
add_executable(${EXECUTABLE_NAME} ${EXAMPLE_SOURCES})

# Link with the simple-framework and OpenGL ES libraries.
set(ALL_LIBRARIES simple-framework2 ${OPENGLES_LIBRARIES})
target_link_libraries(${EXECUTABLE_NAME} ${ALL_LIBRARIES})

# If we compile for ARM, we add the link flag -lm
if (${TARGET} STREQUAL "ARM")
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-lm")
endif()

# If we compile for x86-Linux, we link with X11 libraries
if(${TARGET} STREQUAL "LINUX")
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32 -lX11")
endif()

set(EXECUTABLE_DESTINATION "OpenGL-ES-2.0/${EXECUTABLE_NAME}")

# Install the executable in the package folder.
install(TARGETS ${EXECUTABLE_NAME} 
        RUNTIME DESTINATION ${EXECUTABLE_DESTINATION}
        COMPONENT ${EXECUTABLE_NAME})
            
# Copy the assets folder in install directory.
install(DIRECTORY assets DESTINATION ${EXECUTABLE_DESTINATION})