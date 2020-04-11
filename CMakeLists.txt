cmake_minimum_required(VERSION 3.1)

project(slowmoVideo)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

set(CMAKE_CXX_STANDARD 14)

# Partly from https://doc.qt.io/qt-5/cmake-get-started.html#build-a-gui-executable
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


set(CMAKE_MODULE_PATH
  ${slowmoVideo_SOURCE_DIR}/cmake
)

# Make a version file containing the current version from git.
#
include(GetGitRevisionDescription)
git_describe(VERSION --dirty=-dev)
if (VERSION)
				#parse the version information into pieces.
			  # v0.4.0-123-gdddf621
				string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" PROJECT_VERSION_MAJOR "${VERSION}")
				string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" PROJECT_VERSION_MINOR "${VERSION}")
				string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+)-.*" "\\1" PROJECT_VERSION_PATCH "${VERSION}")
				string(REGEX REPLACE "^v[0-9]+\\.[0-9]+-([0-9]+)-.*" "\\1" PROJECT_VERSION_PATCH "${VERSION}")
				string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-(.*)" "\\1" PROJECT_VERSION_SHA1 "${VERSION}")
else()
				set(PROJECT_VERSION_MAJOR "0")
				set(PROJECT_VERSION_MINOR "5")
				set(PROJECT_VERSION_PATCH "0")
endif()

if(NOT PROJECT_VERSION_PATCH)
				# git describe bug ?
				set(PROJECT_VERSION_PATCH "0")
endif()

set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
configure_file(src/version.h.in version.h)

### Compiler options ###

if (APPLE)
# To compile with clang:
#set(CMAKE_CXX_COMPILER "clang++")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --verbose")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -mtune=corei7")

# Set additional project information
set(COMPANY "granjow")
set(COPYRIGHT "Copyright (c) 2011 Simon A. Eugster (Granjow). All rights reserved.")
set(IDENTIFIER "net.granjow.slomoui")

else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
endif()

if(CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMXE")
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DMXE")
endif(CMAKE_TOOLCHAIN_FILE)


### CMake Configuration ###
option (ENABLE_TESTS "Build the unit tests" FALSE)
set(ADDITIONAL_LIBS "")
if(MSYS)
  message(STATUS "MSYS system detected.")
  include("${PROJECT_SOURCE_DIR}/cmake/MingwCrossEnv.cmake")
endif(MSYS)


### Find packages ###


# Check if environment variable QTDIR is set.
# needed for Qt5
# Extra security for windows environment as well.
if (DEFINED ENV{QTDIR})
    set(CMAKE_PREFIX_PATH $ENV{QTDIR} ${CMAKE_PREFIX_PATH})
endif ()

if (APPLE)
				set(DEST "slowmoUI.app/Contents/Tools/bin")
else()
				set(DEST "bin")
endif()

include(cmake/macros.cmake)

find_package(Qt5Core QUIET)
if (Qt5Core_FOUND)
	message(STATUS "Using Qt5")
	set(USE_QT TRUE)
	# go on with other packages
	find_package(Qt5 COMPONENTS Core Widgets Gui Xml Script REQUIRED)
	if (Qt5_POSITION_INDEPENDENT_CODE)
		set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	endif (Qt5_POSITION_INDEPENDENT_CODE)
	#		 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
	#		    ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

	include_directories(${Qt5Core_INCLUDES})
	include_directories(${Qt5Widgets_INCLUDES})
	include_directories(${Qt5Gui_INCLUDES})
	include_directories(${Qt5Xml_INCLUDES})
	include_directories(${Qt5Script_INCLUDES})

	# set up a mapping so that the Release configuration for the Qt imported target is
	# used in the COVERAGE CMake configuration.
	#set_target_properties(Qt5::Core PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "DEBUG")

endif (Qt5Core_FOUND)


message("Qt libraries found at : ${Qt5Gui_LIBRARIES} /  ${QT_LIBRARIES}" )

set (USE_QTKIT OFF CACHE BOOL "Build with the QTKit encoder")
set (USE_FFMPEG ON CACHE BOOL "Build with the FFMPEG encoder")
set (USE_DBUS OFF CACHE BOOL "Build with the DBUS notification support")

if(NOT MSYS)
	find_package(FFMPEG)
else(NOT MSYS)
    # Handled by MingwCrossEnv.cmake to avoid errors like:
    # libavformat.a(avisynth.o):avisynth.c:(.text+0x6b): undefined reference to `AVIStreamRelease@4'
endif(NOT MSYS)
# not here anymore
#include_directories(${FFMPEG_INCLUDE_DIR})
#include_directories("/usr/include/ffmpeg/")
#link_directories(${FFMPEG_LIBRARY_DIR})

if (APPLE AND USE_QTKIT)
	find_package(QTKIT)
	message(STATUS "QTKIT find at ${QTKIT_LIBRARY} ")
	set(ADDITIONAL_LIBS "-framework Cocoa -framework QTKit -framework QuartzCore -framework AppKit -framework OpenCL")
endif()
	
# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED)
# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file. 
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS " version: ${OpenCV_VERSION}")
message(STATUS " libraries: ${OpenCV_LIBS}")
message(STATUS " include path: ${OpenCV_INCLUDE_DIRS}")

if (${OpenCV_VERSION_MAJOR} EQUAL 3)
				set(HAS_OCV_VERSION_3 ON)
else()
				set(HAS_OCV_VERSION_3 OFF)
endif()

include_directories(${OPENCV_INCLUDE_DIRS})

# for config.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})



### Set up libraries ###
if(MSYS)
    set(EXTERNAL_LIBS ${FFMPEG_LIBRARIES} ${QT_LIBRARIES} ${OpenCV_LIBS_OPT} ${OpenCV_EXTRA_LIBS_OPT} ${ADDITIONAL_LIBS})
else(MSYS)
    set(EXTERNAL_LIBS ${QT_LIBRARIES} ${OpenCV_LIBS} ${ADDITIONAL_LIBS} ${FFMPEG_LIBRARIES})
endif(MSYS)


### Information output
set(BUILD_SLOWMO "NO")
#if(QT_LIBRARIES AND FFMPEG_FOUND)
if(USE_QT AND FFMPEG_FOUND)
set(BUILD_SLOWMO "YES")
#endif(QT_LIBRARIES AND FFMPEG_FOUND)
endif()
if(NOT FFMPEG_SWSCALE_FOUND)
  if(CMAKE_TOOLCHAIN_FILE)

  else(CMAKE_TOOLCHAIN_FILE)
  set(BUILD_SLOWMO "NO")
  endif(CMAKE_TOOLCHAIN_FILE)
endif(NOT FFMPEG_SWSCALE_FOUND)


## Include projects to build ##

include_directories(src/tr)
add_subdirectory(src/lib)
add_subdirectory(src/lib/libsvflow)
add_subdirectory(src/libgui)
add_subdirectory(src/project)
add_subdirectory(src/slowmoCLI)
add_subdirectory(src/slowmoUI)
add_subdirectory(src/slowmoFlowEdit)
add_subdirectory(src/slowmoInfo)
add_subdirectory(src/slowmoRenderer)
add_subdirectory(src/visualizeFlow)
if(ENABLE_TESTS)
  SET(QT_USE_QTTEST TRUE)
  ## add_subdirectory(slowmoVideo/test)
  add_subdirectory(slowmoVideo/unittests)
endif(ENABLE_TESTS)

##### SV END #####






message("==================slowmoVideo========================")
message("* (info) slowmoVideo installation goes to ${CMAKE_INSTALL_PREFIX}.")
message("         (Can be adjusted with -DCMAKE_INSTALL_PREFIX=your_path. Default is ${SV_INST_DIR}.)")
#if(NOT QT_LIBRARIES)
if (NOT USE_QT)
    message("Qt5 libraries could not be found.")
#endif(NOT QT_LIBRARIES)
endif(NOT USE_QT)
if(NOT FFMPEG_FOUND)
    message("x ffmpeg libraries could not be found.")
else(NOT FFMPEG_FOUND)
    message("* (ok) ffmpeg found at ${FFMPEG_LIBRARY_DIR}")
endif(NOT FFMPEG_FOUND)
if(NOT FFMPEG_SWSCALE_FOUND)
    message("x libswscale could not be found.")
endif(NOT FFMPEG_SWSCALE_FOUND)
if(NOT OpenCV_VERSION)
    message("x OpenCV could not be found.")
else(NOT OpenCV_VERSION)
    message("* (ok) OpenCV ${OpenCV_VERSION} found at ${OpenCV_INCLUDE_DIRS}.")
endif(NOT OpenCV_VERSION)
message("* slowmoVideo will be built:                ---${BUILD_SLOWMO}---")
message("=======================END===========================")
message("")


if(NOT BUILD_SLOWMO)
    error(WARNING "Cannot build slowmoVideo, please install the missing packages first.")
endif(NOT BUILD_SLOWMO)

configure_file(src/config.h.in config.h)
