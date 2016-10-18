set(SRCS
    test.cpp
)

include_directories(${FFMPEG_INCLUDE_PATHS})
add_executable(Test ${SRCS})
target_link_libraries(Test sVproj ${EXTERNAL_LIBS})

#add_executable(encodeTest encodeTest.c)
#target_link_libraries(encodeTest sVencode ${EXTERNAL_LIBS})

#add_executable(encodeFramesTest ffmpegTestEncodeFrames.cpp)
#message(STATUS "Additional libraries: ${ADDITIONAL_LIBS}")
#target_link_libraries(encodeFramesTest sVencode  ${EXTERNAL_LIBS})

add_executable(AvconvInfo testAvconvInfo.cpp)
target_link_libraries(AvconvInfo sVinfo ${EXTERNAL_LIBS})

install(TARGETS Test DESTINATION bin)
