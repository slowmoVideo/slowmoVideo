cmake_minimum_required(VERSION 2.8)

project(slowmoVideo)

set(CMAKE_BUILD_TYPE Debug)

# To compile with clang:
#set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_FLAGS
  ${CMAKE_CXX_FLAGS}
  -Wall
)

set(SOURCES
  main.cpp
  interpolateSV.cpp
)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDES})



macro(copy_files FILELIST)
  foreach (FILENAME ${FILELIST})
    set(FILE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
    set(FILE_DST "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}")
    message("   Copying " ${FILENAME} " to " ${FILE_DST})

    configure_file(${FILE_SRC} ${FILE_DST} COPYONLY)
  endforeach(FILENAME)
endmacro(copy_files)

copy_files(
  fetch.sh
)

add_executable(slowmoVideo ${SOURCES})
target_link_libraries(slowmoVideo ${QT_LIBRARIES})
