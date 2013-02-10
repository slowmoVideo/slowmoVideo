# Docs: http://www.cmake.org/cmake/help/cmake2.6docs.html

cmake_minimum_required(VERSION 2.6)

project(slowmoVideo)


### Compiler options ###

# To compile with clang:
#set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -mtune=corei7")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --verbose")
if(CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMXE")
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DMXE")
endif(CMAKE_TOOLCHAIN_FILE)


### CMake Configuration ###
option (ENABLE_TESTS "Build the unit tests" TRUE)
set(ADDITIONAL_LIBS "")
if(MSYS)
  message(STATUS "MSYS system detected.")
  include("${PROJECT_SOURCE_DIR}/cmake/MingwCrossEnv.cmake")
endif(MSYS)


### Find packages ###

set(CMAKE_MODULE_PATH
  ${slowmoVideo_SOURCE_DIR}/cmake
)

include(cmake/macros.cmake)

SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTTEST TRUE)
SET(QT_USE_QTSCRIPT TRUE)
find_package(Qt4)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDES})


if(NOT MSYS)
	find_package(FFMPEG)
else(NOT MSYS)
    # Handled by MingwCrossEnv.cmake to avoid errors like:
    # libavformat.a(avisynth.o):avisynth.c:(.text+0x6b): undefined reference to `AVIStreamRelease@4'
endif(NOT MSYS)
include_directories(${FFMPEG_INCLUDE_DIR})
include_directories("/usr/include/ffmpeg/")
link_directories(${FFMPEG_LIBRARY_DIR})

find_package(OpenCV)
include_directories(${OPENCV_INCLUDE_DIRS})



### Set up libraries ###
if(MSYS)
    set(EXTERNAL_LIBS ${FFMPEG_LIBRARIES} ${QT_LIBRARIES} ${OpenCV_LIBS_OPT} ${OpenCV_EXTRA_LIBS_OPT} ${ADDITIONAL_LIBS})
else(MSYS)
    set(EXTERNAL_LIBS ${FFMPEG_LIBRARIES} ${QT_LIBRARIES} ${OpenCV_LIBS} ${ADDITIONAL_LIBS})
endif(MSYS)


### Information output
set(BUILD_SLOWMO "NO")
if(QT_LIBRARIES AND FFMPEG_FOUND)
set(BUILD_SLOWMO "YES")
endif(QT_LIBRARIES AND FFMPEG_FOUND)
if(NOT FFMPEG_SWSCALE_FOUND)
  if(CMAKE_TOOLCHAIN_FILE)

  else(CMAKE_TOOLCHAIN_FILE)
  set(BUILD_SLOWMO "NO")
  endif(CMAKE_TOOLCHAIN_FILE)
endif(NOT FFMPEG_SWSCALE_FOUND)

message("==================================================")
message("* slowmoVideo installation goes to ${CMAKE_INSTALL_PREFIX}.")
message("  (Can be adjusted with the INSTALL_PREFIX variable (not CMAKE_INSTALL_PREFIX). Default is ${SV_INST_DIR}.)")
if(NOT QT_LIBRARIES)
message("x Qt4 libraries could not be found.")
endif(NOT QT_LIBRARIES)
if(NOT FFMPEG_FOUND)
message("x ffmpeg libraries could not be found.")
else(NOT FFMPEG_FOUND)
message("* ffmpeg found at ${FFMPEG_LIBRARY_DIR}")
endif(NOT FFMPEG_FOUND)
if(NOT FFMPEG_SWSCALE_FOUND)
message("x libswscale could not be found.")
endif(NOT FFMPEG_SWSCALE_FOUND)
if(NOT OpenCV_VERSION)
message("x OpenCV could not be found.")
else(NOT OpenCV_VERSION)
message("* OpenCV ${OpenCV_VERSION} found at ${OPENCV_LIB_DIR}.")
endif(NOT OpenCV_VERSION)
message("* slowmoVideo will be built: ${BUILD_SLOWMO}")
message("==================================================")


if(NOT BUILD_SLOWMO)
message(FATAL_ERROR "Cannot build slowmoVideo, please install the missing packages first.")
endif(NOT BUILD_SLOWMO)





## Include projects to build ##

include_directories(tr)
add_subdirectory(lib)
add_subdirectory(libgui)
add_subdirectory(project)
add_subdirectory(slowmoCLI)
add_subdirectory(slowmoUI)
add_subdirectory(slowmoFlowEdit)
add_subdirectory(slowmoInfo)
add_subdirectory(slowmoRenderer)
add_subdirectory(visualizeFlow)
if(ENABLE_TESTS)
add_subdirectory(test)
add_subdirectory(unittests)
endif(ENABLE_TESTS)
