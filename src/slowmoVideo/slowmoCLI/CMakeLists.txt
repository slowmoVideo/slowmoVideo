

set(SOURCES_MAIN
  main.cpp
)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCES_VINFO
  videoInfo.cpp
)

add_executable(slowmoInterpolate ${SOURCES_MAIN})
target_link_libraries(slowmoInterpolate sV sVflow )

add_executable(slowmoVideoInfo ${SOURCES_VINFO})
target_link_libraries(slowmoVideoInfo sVinfo )

install(TARGETS slowmoInterpolate DESTINATION ${DEST})
install(TARGETS slowmoVideoInfo DESTINATION ${DEST})
