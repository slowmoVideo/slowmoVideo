

set(SOURCES_MAIN
  main.cpp
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCES_VINFO
  videoInfo.cpp
)

add_executable(slowmoInterpolate ${SOURCES_MAIN})
target_link_libraries(slowmoInterpolate sV sVflow ${EXTERNAL_LIBS})

add_executable(videoInfo ${SOURCES_VINFO})
target_link_libraries(videoInfo sVinfo ${EXTERNAL_LIBS})

install(TARGETS slowmoInterpolate DESTINATION bin)
install(TARGETS videoInfo DESTINATION bin)
