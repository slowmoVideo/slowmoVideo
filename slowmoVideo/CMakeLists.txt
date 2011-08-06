# Docs: http://www.cmake.org/cmake/help/cmake-2-8-docs.html

cmake_minimum_required(VERSION 2.8)

project(slowmoVideo)



# Installation prefix
# Thanks to David Cole: http://www.mail-archive.com/cmake@cmake.org/msg07872.html
get_filename_component(SV_INST_DIR "${slowmoVideo_SOURCE_DIR}/../install/" ABSOLUTE)
set(INSTALL_PREFIX ${SV_INST_DIR} CACHE PATH "Installation prefix (CMAKE_INSTALL_PREFIX will NOT work)")
set(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX} CACHE INTERNAL "Install prefix" FORCE)


## Compiler options ##

# To compile with clang:
#set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS
  ${CMAKE_CXX_FLAGS}
  -Wall
)

## Find packages ##

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

find_package(FFMPEG)


set(BUILD_SLOWMO "NO")
if(QT_LIBRARIES AND FFMPEG_FOUND AND FFMPEG_SWSCALE_FOUND)
set(BUILD_SLOWMO "YES")
endif(QT_LIBRARIES AND FFMPEG_FOUND AND FFMPEG_SWSCALE_FOUND)

message("==================================================")
message("* slowmoVideo installation goes to ${CMAKE_INSTALL_PREFIX}.")
message("  (Can be adjusted with the INSTALL_PREFIX variable (not CMAKE_INSTALL_PREFIX). Default is ${SV_INST_DIR}.)")
if(NOT QT_LIBRARIES)
message("* Qt4 libraries could not be found.")
endif(NOT QT_LIBRARIES)
if(NOT FFMPEG_FOUND)
message("* ffmpeg libraries could not be found.")
endif(NOT FFMPEG_FOUND)
if(NOT FFMPEG_SWSCALE_FOUND)
message("* libswscale could not be found.")
endif(NOT FFMPEG_SWSCALE_FOUND)
message("* slowmoVideo will be built: ${BUILD_SLOWMO}")
message("==================================================")


if(NOT BUILD_SLOWMO)
message(FATAL_ERROR "Cannot build slowmoVideo, please install the missing packages first.")
endif(NOT BUILD_SLOWMO)


## Include projects to build ##

add_subdirectory(lib)
add_subdirectory(libgui)
add_subdirectory(project)
add_subdirectory(slowmoCLI)
add_subdirectory(slowmoUI)
add_subdirectory(slowmoFlowEdit)
add_subdirectory(slowmoInfo)
add_subdirectory(slowmoRenderer)
add_subdirectory(visualizeFlow)
add_subdirectory(test)
add_subdirectory(unittests)
