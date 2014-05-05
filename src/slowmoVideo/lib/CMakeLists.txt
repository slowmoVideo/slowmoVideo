# Static libraries
# http://www.itk.org/pipermail/insight-users/2007-November/024141.html
# http://www.linux-magazin.de/Heft-Abo/Ausgaben/2007/02/Mal-ausspannen

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(LIB_SRC_BASE
  defs_sV.cpp
  defs_sV.hpp
  vector_sV.cpp
  shutter_sV.cpp
  intMatrix_sV.cpp
  interpolate_sV.cpp
  bezierTools_sV.cpp
  sourceField_sV.cpp
)

set(LIB_SRC_VIDEO
  defs_sV.h
  videoInfo_sV.c
  avconvInfo_sV.cpp
)

set(LIB_SRC_ARGS
  trivialArgsReader_sV.cpp
)

set(LIB_SRC_FLOW
  flowRW_sV.cpp
  flowField_sV.cpp
  flowTools_sV.cpp
  kernel_sV.cpp
)

set(LIB_SRC_FLOWVIS
  flowVisualization_sV.cpp
)

set(HEADERS
  flowRW_sV.h
  flowField_sV.h
  flowTools_sV.h
)


set(LIB_SRC_ENCODE
  macros_sV.h
  video_enc.h
  ffmpeg_writer.h
  ffmpegEncode_sV.c
  ffmpeg_writer.cpp
  video_enc.cpp
)

if (APPLE AND USE_QTKIT)
set(LIB_SRC_ENCODE
	${LIB_SRC_ENCODE}
	qtkit.h
	qtkit.mm
)
endif()

message(STATUS "FFMPEG libraries are at ${FFMPEG_LIBRARIES}")

include_directories(${FFMPEG_INCLUDE_PATHS})
include_directories(${Qt5Core_INCLUDE_DIRS})
add_library(sV  STATIC ${LIB_SRC_BASE})
target_link_libraries(sV  ${QT_LIBRARIES})

add_library(sVinfo  STATIC ${LIB_SRC_VIDEO})
target_link_libraries(sVinfo  ${FFMPEG_LIBRARIES})

add_library(sVencode  STATIC ${LIB_SRC_ENCODE})
target_link_libraries(sVencode  ${FFMPEG_LIBRARIES})

add_library(sVflow  STATIC ${LIB_SRC_FLOW})

add_library(sVvis  STATIC ${LIB_SRC_FLOWVIS})
target_link_libraries(sVvis  sVflow ${QT_LIBRARIES})

