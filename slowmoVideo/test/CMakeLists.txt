set(CMAKE_C_FLAGS
  ${CMAKE_C_FLAGS}
  -std=c99
)
message(STATUS "CXX flags: ${CMAKE_CXX_FLAGS}")

set(SRCS
    test.cpp
)

add_executable(Test ${SRCS})
target_link_libraries(Test sVproj ${QT_LIBRARIES})

add_executable(encodeTest encodeTest.c)
target_link_libraries(encodeTest sVencode ${FFMPEG_LIBRARIES})

add_executable(encodeFramesTest ffmpegTestEncodeFrames.cpp)
target_link_libraries(encodeFramesTest sVencode ${FFMPEG_LIBRARIES} ${QT_LIBRARIES})

install(TARGETS Test DESTINATION bin)
