@ECHO OFF
REM This confidential and proprietary software may be used only as
REM authorised by a licensing agreement from ARM Limited
  REM (C) COPYRIGHT 2012 ARM Limited ALL RIGHTS RESERVED
REM The entire notice above must be reproduced on all authorised
REM copies and copies may only be made to the extent permitted
REM by a licensing agreement from ARM Limited.

REM Build all the samples for x86 Windows,
REM running on the OpenGL ES Emulator.

REM Enable delayed variable expansion same as cmd.exe /V
setlocal ENABLEDELAYEDEXPANSION

REM Save the working directory
set ROOTDIRECTORY="%CD%"

REM Directory that contains the OpenGL ES 2.0 Linux samples sources.
set OPENGL_ES2_DIRECTORY=samples\OpenGL-ES-2.0\linux

REM Prepare the list of OpenGL ES 2.0 sample directories 
set OPENGL_ES2_SAMPLES=
cd %OPENGL_ES2_DIRECTORY%
for /d %%i in (*) do set OPENGL_ES2_SAMPLES=!OPENGL_ES2_SAMPLES! %%i
cd %ROOTDIRECTORY%

REM Directory that contains the OpenGL ES 3.0 Linux samples sources.
set OPENGL_ES3_DIRECTORY=samples\OpenGL-ES-3.0\linux

REM Prepare the list of OpenGL ES 3.0 sample directories 
set OPENGL_ES3_SAMPLES=
cd %OPENGL_ES3_DIRECTORY%
for /d %%i in (*) do set OPENGL_ES3_SAMPLES=!OPENGL_ES3_SAMPLES! %%i
cd %ROOTDIRECTORY%

REM Directory that will contain the CMake build files and generated makefiles.
set CMAKE_BUILD_DIRECTORY=build\cmake-files-x86

set IF_RESULT=false

REM Check the command-line arguments.
if "%1" == "--help" (
	set IF_RESULT=true
)

if "%1" == "/?" (
	set IF_RESULT=true
)

if %IF_RESULT% == true (
    echo Build samples for Windows. v2.0.0
	echo Usage: build-x86-win32.bat ^<sample_name^>
    echo Set a ^<sample_name^> argument to build one sample for x86 system.
    echo The executable is built inside: build\x86\
    echo.
	echo Valid OpenGL ES 2.0 sample names are:
	echo %OPENGL_ES2_SAMPLES%
    echo.
	echo Valid OpenGL ES 3.0 sample names are:
	echo %OPENGL_ES3_SAMPLES%
	echo.
	echo If ^<sample_name^> is empty all examples are built inside: build\x86\
	echo. 
	echo Ensure your are running in a Visual Studio Command Prompt and your PATH contains CMake.
	goto END
)

REM Check if the directory already exists before creating it
if NOT EXIST %CMAKE_BUILD_DIRECTORY% (
    REM Creating the build directories.
    echo Creating %CMAKE_BUILD_DIRECTORY% directories...
    mkdir %CMAKE_BUILD_DIRECTORY%
)

REM Move inside the build directory.
cd %CMAKE_BUILD_DIRECTORY%

echo === BUILDING FOR x86 ON EMULATOR ===

REM First stage of the CMake build process.
REM NMake Makefiles are generated.
REM CMake is reading CMakeLists.txt files starting from main SDK directory,
REM and it is generating makefiles inside the CMAKE_BUILD_DIRECTORY.
if "%1" == "--release" (
    cmake -G"NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..\..\
	REM Move on to the next argument now ignoring the first one
	shift
) else (
    cmake -G"NMake Makefiles" ..\..\
)

REM Check if the command-line argument is set.
if NOT "%1" == "" (  
    REM Look for the specified sample in the OpenGL ES 2.0 and 3.0 folders,
    REM then build it.
    cd %OPENGL_ES2_DIRECTORY%
    for /d %%q in (*) do (
        if "%%q" == "%1" (
            cd %1
            nmake install
            goto ENDIF
            )
    )
    
    cd %ROOTDIRECTORY%\%CMAKE_BUILD_DIRECTORY%\%OPENGL_ES3_DIRECTORY%
    for /d %%q in (*) do (
        if "%%q" == "%1" (
            cd %1
            nmake install
            goto ENDIF
            )
    )
    
    echo.
    echo Sample %1 not found. Run 'build-x86-win32.bat --help' to see list of available samples.
    echo.
) else (
    REM Run the Make command from the root to build all the examples.
    nmake install
)

:ENDIF

REM Jump back to the root
cd %ROOTDIRECTORY%

:END