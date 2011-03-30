cmake_minimum_required(VERSION 2.8)

project(slowmoVideo)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS
  ${CMAKE_CXX_FLAGS}
  -Wall
)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDES})


add_executable(slowmoVideo main.cpp)
target_link_libraries(slowmoVideo ${QT_LIBRARIES})
