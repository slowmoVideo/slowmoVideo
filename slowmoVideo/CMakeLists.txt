# Docs: http://www.cmake.org/cmake/help/cmake-2-8-docs.html

cmake_minimum_required(VERSION 2.8)

project(slowmoVideo)


## Compiler options ##

# To compile with clang:
#set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS
  ${CMAKE_CXX_FLAGS}
  -Wall
)

## Builder options ##

get_filename_component(CMAKE_INSTALL_PREFIX "${slowmoVideo_SOURCE_DIR}/../install" ABSOLUTE)
message("Installing to ${CMAKE_INSTALL_PREFIX}")
set(CMAKE_MODULE_PATH
  ${slowmoVideo_SOURCE_DIR}/cmake
)


## Find packages ##

include(cmake/macros.cmake)

SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTTEST TRUE)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDES})

find_package(FFMPEG REQUIRED)


## Include projects ##

add_subdirectory(lib)
add_subdirectory(project)
add_subdirectory(slowmoCLI)
add_subdirectory(slowmoUI)
add_subdirectory(visualizeFlow)
add_subdirectory(test)
add_subdirectory(unittests)




install(FILES fetch.sh DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
