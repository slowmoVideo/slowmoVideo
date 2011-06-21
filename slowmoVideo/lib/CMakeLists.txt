# Static libraries
# http://www.itk.org/pipermail/insight-users/2007-November/024141.html
# http://www.linux-magazin.de/Heft-Abo/Ausgaben/2007/02/Mal-ausspannen

set(CMAKE_C_FLAGS
  ${CMAKE_C_FLAGS}
  -std=c99
)

set(LIB_SRC
  defs_sV.cpp
  defs_sV.hpp
  interpolate_sV.cpp
)

set(LIB_SRC_VI
  defs_sV.h
  videoInfo_sV.c
)

set(LIB_SRC_ARGS
  trivialArgsReader_sV.cpp
)

set(LIB_SRC_FLOW
  flowRW_sV.cpp
  flowField_sV.cpp
)

set(HEADERS
  flowRW_sV.h
  flowField_sV.h
)



add_library(sV STATIC ${LIB_SRC})
target_link_libraries(sV ${QT_LIBRARIES})

add_library(sVinfo STATIC ${LIB_SRC_VI})
target_link_libraries(sVinfo ${FFMPEG_LIBRARIES})

add_library(sVflow STATIC ${LIB_SRC_FLOW})
install(TARGETS sVflow DESTINATION "lib")
install(FILES ${HEADERS} DESTINATION "include")
