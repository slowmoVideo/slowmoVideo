# Static libraries
# http://www.itk.org/pipermail/insight-users/2007-November/024141.html
# http://www.linux-magazin.de/Heft-Abo/Ausgaben/2007/02/Mal-ausspannen

set(CMAKE_C_FLAGS
  ${CMAKE_C_FLAGS}
  -std=c99
)

set(LIB_SRC
  interpolate_sV.cpp
)

set(LIB_SRC_VI
  videoInfo_sV.c
)



add_library(sV STATIC ${LIB_SRC})
target_link_libraries(sV ${QT_LIBRARIES})

add_library(sVI STATIC ${LIB_SRC_VI})
target_link_libraries(sVI ${FFMPEG_LIBRARIES})
