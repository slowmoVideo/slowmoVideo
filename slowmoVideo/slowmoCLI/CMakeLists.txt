

set(SOURCES_MAIN
  main.cpp
)

set(CMAKE_C_FLAGS
  -std=c99
)
set(SOURCES_VINFO
  videoInfo.cpp
  ../lib/videoInfo_sV.c
)

add_executable(slowmoVideo ${SOURCES_MAIN})
target_link_libraries(slowmoVideo sV ${QT_LIBRARIES})

add_executable(videoInfo ${SOURCES_VINFO})
target_link_libraries(videoInfo sVI)

install(TARGETS slowmoVideo DESTINATION bin)
install(TARGETS videoInfo DESTINATION bin)
