cmake_minimum_required(VERSION 2.6)
project(slowmoVideo)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

set (CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

#
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
configure_file(version.h.in version.h)

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

# search for Qt4
if (NOT FORCE_QT4)
         find_package(Qt5Core QUIET)
         if (Qt5Core_FOUND) 
                 message(STATUS "Using qt5")
		 set(USE_QT TRUE)
		 # go on with other packages
		 find_package(Qt5 COMPONENTS Core Widgets Gui Xml Script)
		if (Qt5_POSITION_INDEPENDENT_CODE)
			set(CMAKE_POSITION_INDEPENDENT_CODE ON)
		endif(Qt5_POSITION_INDEPENDENT_CODE)
#		 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
#		    ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

		 include_directories(${Qt5Core_INCLUDES}) 
		 include_directories(${Qt5Widgets_INCLUDES}) 
		 include_directories(${Qt5Gui_INCLUDES}) 
		 include_directories(${Qt5Xml_INCLUDES}) 
		 include_directories(${Qt5Script_INCLUDES}) 

		 macro(qt_use_modules)
		    qt5_use_modules(${ARGN})
		 endmacro()
 
		 macro(qt_wrap_ui)
		     qt5_wrap_ui(${ARGN})
		 endmacro()
 
		 macro(qt_wrap_cpp)
		     qt5_wrap_cpp(${ARGN})
		 endmacro()
 
		 macro(qt_add_resources)
		     qt5_add_resources(${ARGN})
		 endmacro()
 
		 #find_package(Qt5LinguistTools REQUIRED)
		 macro(qt_add_translation)
		     qt5_add_translation(${ARGN})
		 endmacro()

		 macro(install_qt_executable)
			install_qt5_executable(${ARGN})
		 endmacro()
	 endif(Qt5Core_FOUND)
endif(NOT FORCE_QT4)

#old qt4 ...
if(NOT Qt5Core_FOUND) 

# should replace :
#SET(QT_USE_QTXML TRUE)
#SET(QT_USE_QTSCRIPT TRUE)
#find_package(Qt4)
#include(${QT_USE_FILE})
#include_directories(${QT_INCLUDES})

     message(STATUS "Could not find Qt5, searching for Qt4 instead...")
 
     set(NEEDED_QT4_COMPONENTS "QtCore" "QtXml" "QtGui" "QtScript") 
 
     find_package(Qt4 REQUIRED COMPONENTS ${NEEDED_QT4_COMPONENTS})

     macro(qt_use_modules)
 
     endmacro()
 
     macro(qt_wrap_ui)
         qt4_wrap_ui(${ARGN})
     endmacro()
 
     macro(qt_wrap_cpp)
         qt4_wrap_cpp(${ARGN})
     endmacro()
 
     macro(qt_add_resources)
         qt4_add_resources(${ARGN})
     endmacro()
 
     macro(qt_add_translation)
         qt4_add_translation(${ARGN})
     endmacro()
 
     macro(install_qt_executable)
         install_qt4_executable(${ARGN})
     endmacro()

     include(${QT_USE_FILE})
     add_definitions(${QT_DEFINITIONS})
 
     set(USE_QT TRUE)
endif()

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

include_directories(slowmoVideo/tr)
add_subdirectory(slowmoVideo/lib)
add_subdirectory(slowmoVideo/libgui)
add_subdirectory(slowmoVideo/project)
add_subdirectory(slowmoVideo/slowmoCLI)
add_subdirectory(slowmoVideo/slowmoUI)
add_subdirectory(slowmoVideo/slowmoFlowEdit)
add_subdirectory(slowmoVideo/slowmoInfo)
add_subdirectory(slowmoVideo/slowmoRenderer)
add_subdirectory(slowmoVideo/visualizeFlow)
if(ENABLE_TESTS)
  SET(QT_USE_QTTEST TRUE)
  ## add_subdirectory(slowmoVideo/test)
  add_subdirectory(slowmoVideo/unittests)
endif(ENABLE_TESTS)

##### SV END #####





##### V3D START #####

if(WIN32)
set(GLUT_ROOT_PATH ${PROJECT_SOURCE_DIR}/libs/)
endif(WIN32)

find_package(OpenGL)
find_package(GLEW)
find_package(GLUT)
find_package(JPEG)
find_package(PNG)
find_package(ZLIB)
#find_package(X11)

# Windows: Try to find libraries that could not be found manually in the libs/ directory.
if(WIN32)
	if(NOT ZLIB_FOUND)
		FIND_PATH(ZLIB_INCLUDE_DIR zlib.h
			PATHS ${PROJECT_SOURCE_DIR}/libs/include
		)
		find_library(ZLIB_LIBRARY NAMES zlib PATHS ${PROJECT_SOURCE_DIR}/libs/lib)
		if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
			set(ZLIB_FOUND TRUE)
		endif(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
		message(STATUS "Manual search for zlib: ${ZLIB_LIBRARY} in ${ZLIB_INCLUDE_DIR}")
	endif(NOT ZLIB_FOUND)
	
	if(NOT PNG_FOUND)
		find_path(PNG_INCLUDE_DIR png.h PATHS ${PROJECT_SOURCE_DIR}/libs/include)
		find_library(PNG_LIBRARIES libpng PATHS ${PROJECT_SOURCE_DIR}/libs/lib)
		if(PNG_INCLUDE_DIR AND PNG_LIBRARIES)
			set(PNG_FOUND TRUE)
		endif(PNG_INCLUDE_DIR AND PNG_LIBRARIES)
		message(STATUS "Manual search for png: ${PNG_LIBRARIES} in ${PNG_INCLUDE_DIR}")
	endif(NOT PNG_FOUND)
	
	if(NOT GLUT_FOUND)
		find_path(GLUT_LIBRARY_DIR NAMES GL/glut.h PATHS ${PROJECT_SOURCE_DIR}/libs/include)
		find_library(GLUT_LIBRARIES glut32 PATHS ${PROJECT_SOURCE_DIR}/libs/lib)
		if(GLUT_LIBRARY_DIR AND GLUT_LIBRARIES)
			set(GLUT_FOUND TRUE)
		endif(GLUT_LIBRARY_DIR AND GLUT_LIBRARIES)
		message(STATUS "Manual search for GLUT: ${GLUT_LIBRARIES} in ${GLUT_LIBRARY_DIR}")
	endif(NOT GLUT_FOUND)

	if(NOT JPEG_FOUND)
		FIND_PATH(JPEG_INCLUDE_DIR jpeglib.h PATHS ${PROJECT_SOURCE_DIR}/libs/include)

		SET(JPEG_NAMES ${JPEG_NAMES} jpeg)
		FIND_LIBRARY(JPEG_LIBRARY NAMES ${JPEG_NAMES} PATHS ${PROJECT_SOURCE_DIR}/libs/lib)
		if(JPEG_INCLUDE_DIR AND JPEG_LIBRARY)
			set(JPEG_FOUND TRUE)
		endif(JPEG_INCLUDE_DIR AND JPEG_LIBRARY)
		message(STATUS "Manual search for JPEG: ${JPEG_LIBRARY} in ${JPEG_INCLUDE_DIR}")
	endif(NOT JPEG_FOUND)
endif(WIN32)


set(BUILD_FLOW_BUILDER "NO")
if(OPENGL_FOUND AND GLUT_FOUND AND GLEW_FOUND AND JPEG_FOUND AND PNG_FOUND)
	set(BUILD_FLOW_BUILDER "YES")
endif(OPENGL_FOUND AND GLUT_FOUND AND GLEW_FOUND AND JPEG_FOUND AND PNG_FOUND)

set(INCLUDE_SOURCE "YES")
if(DISABLE_INCLUDE_SOURCE)
	set(INCLUDE_SOURCE "NO")
add_definitions(-DDISABLE_INCLUDE_SOURCE)
endif(DISABLE_INCLUDE_SOURCE)


if (BUILD_FLOW_BUILDER)
    include_directories(${OPENGL_INCLUDE_DIR})
    include_directories(${GLUT_INCLUDE_DIR})
    include_directories(${GLEW_INCLUDE_DIR})
    include_directories(${JPEG_INCLUDE_DIR})
    include_directories(${ZLIB_INCLUDE_DIR})
    include_directories(${PNG_INCLUDE_DIR})


    set (V3D_DIR ${CMAKE_CURRENT_SOURCE_DIR}/V3D)
    set (V3D_INCLUDE_DIRS ${V3D_DIR}/.)

    include (V3D/Config/v3d_macros.cmake)
    include_directories(${V3D_INCLUDE_DIRS} ${EXTRA_INC_DIRS})

    add_definitions(-DDISABLE_REDEFINITIONS)
    #--------------------------------------------------
    enable_feature (V3DLIB_ENABLE_LIBJPEG)
    enable_feature (V3DLIB_ENABLE_LIBPNG)
    enable_feature_libraries (V3DLIB_ENABLE_LIBJPEG ${JPEG_LIBRARIES})
    enable_feature_libraries (V3DLIB_ENABLE_LIBPNG ${PNG_LIBRARIES})

    enable_feature (V3DLIB_ENABLE_GPGPU)
    enable_feature_libraries (V3DLIB_ENABLE_GPGPU ${OPENGL_LIBRARIES})
    enable_feature_libraries (V3DLIB_ENABLE_GPGPU ${GLEW_LIBRARIES})
    enable_feature_libraries (V3DLIB_ENABLE_GPGPU ${GLUT_glut_LIBRARY})
    #--------------------------------------------------


    include_directories(V3D/Config)

    set (GL_SRC
        V3D/GL/glsl_shaders.cpp
        V3D/GL/v3d_gpubase.cpp
        V3D/GL/v3d_gpuflow.cpp
        V3D/GL/v3d_gpucolorflow.cpp
        V3D/GL/v3d_gpupyramid.cpp
    )

    set (ALL_SRC
        ${GL_SRC}
        V3D/Config/config.h
        
        V3D/Base/v3d_image.cpp
        V3D/Base/v3d_imageprocessing.h
        V3D/Base/v3d_exception.h
        V3D/Base/v3d_timer.h
        V3D/Base/v3d_serialization.h
        V3D/Base/v3d_utilities.h
        V3D/Math/v3d_linear.h
        V3D/Math/v3d_linearbase.h
    )

    add_library(V3D STATIC ${ALL_SRC})
    target_link_libraries(V3D ${GLEW_LIBRARIES} X11)
    #install(TARGETS V3D DESTINATION lib)
    add_subdirectory(V3D/Apps)
endif (BUILD_FLOW_BUILDER)

##### V3D END #####





message("")
message("======================V3D============================")
message("* (info) Installation prefix: ${CMAKE_INSTALL_PREFIX}.")
message("         (Can be adjusted with -DCMAKE_INSTALL_PREFIX=your_path.  Default: ${SV_INST_DIR}.)")
message("* (info) Shaders will be included in the binary: ${INCLUDE_SOURCE}")
if(INCLUDE_SOURCE)
	message("         (Can be disabled with the cmake flag -DDISABLE_INCLUDE_SOURCE)")
endif(INCLUDE_SOURCE)


if(NOT OPENGL_FOUND)
	message("* OpenGL could not be found.")
else(NOT OPENGL_FOUND)
	message("* (ok) OpenGL found in ${OPENGL_INCLUDE_DIR}: ${OPENGL_LIBRARIES}")
endif(NOT OPENGL_FOUND)

if(NOT GLUT_FOUND)
	message("* GLUT could not be found.")
else(NOT GLUT_FOUND)
	message("* (ok) GLUT found in ${GLUT_INCLUDE_DIR}: ${GLUT_LIBRARIES}")
endif(NOT GLUT_FOUND)

if(NOT GLEW_FOUND)
	message("* GLEW could not be found.")
else(NOT GLEW_FOUND)
	message("* (ok) GLEW found at ${GLEW_INCLUDE_DIR}")
endif(NOT GLEW_FOUND)

if(NOT JPEG_FOUND)
	message("* JPEG libraries could not be found.")
else(NOT JPEG_FOUND)
	message("* (ok) JPEG libraries found at ${JPEG_INCLUDE_DIR}: ${JPEG_LIBRARIES}")
endif(NOT JPEG_FOUND)

if(NOT PNG_FOUND)
	message("* PNG libraries could not be found.")
else(NOT PNG_FOUND)
	message("* (ok) PNG libraries found at ${PNG_INCLUDE_DIR}")
endif(NOT PNG_FOUND)

message("* V3D will be built:                        ---${BUILD_FLOW_BUILDER}---")


message("==================slowmoVideo========================")
message("* (info) slowmoVideo installation goes to ${CMAKE_INSTALL_PREFIX}.")
message("         (Can be adjusted with -DCMAKE_INSTALL_PREFIX=your_path. Default is ${SV_INST_DIR}.)")
#if(NOT QT_LIBRARIES)
if (NOT USE_QT)
    message("QT5 nor Qt4 libraries could not be found.")
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
    message(WARNING "Cannot build slowmoVideo, please install the missing packages first.")
endif(NOT BUILD_SLOWMO)

if(NOT BUILD_FLOW_BUILDER)
    message(WARNING "Cannot build V3D.")
endif(NOT BUILD_FLOW_BUILDER)

configure_file(config.h.in config.h)

