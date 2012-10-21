
add_definitions(-DWINDOWS)
set(WINDOWS "yes")



# Make QJson build statically (flag QJson patched by cross-env)
# Thanks to Mark Brand for the hint
add_definitions(-DQJSON_STATIC)



# Find the FFMPEG libraries with PkgConfig
# Thanks to Martin Müllenhaupt for the code
message(STATUS "Checking ffmpeg with pkg_check_modules")
find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG_avdevice libavdevice)
SET(FFMPEG_avdevice_LIBRARY ${FFMPEG_avdevice_STATIC_LIBRARIES})

pkg_check_modules(FFMPEG_avfilter libavfilter)
SET(FFMPEG_avfilter_LIBRARY ${FFMPEG_avfilter_STATIC_LIBRARIES})

pkg_check_modules(FFMPEG_avformat libavformat)
SET(FFMPEG_avformat_LIBRARY ${FFMPEG_avformat_STATIC_LIBRARIES})

pkg_check_modules(FFMPEG_avcodec libavcodec)
SET(FFMPEG_avcodec_LIBRARY ${FFMPEG_avcodec_STATIC_LIBRARIES})

pkg_check_modules(FFMPEG_postproc libpostproc)
SET(FFMPEG_postproc_LIBRARY ${FFMPEG_postproc_STATIC_LIBRARIES})

pkg_check_modules(FFMPEG_swscale libswscale)
SET(FFMPEG_swscale_LIBRARY ${FFMPEG_swscale_STATIC_LIBRARIES})

pkg_check_modules(FFMPEG_avutil libavutil)
SET(FFMPEG_avutil_LIBRARY ${FFMPEG_avutil_STATIC_LIBRARIES})

SET(FFMPEG_INCLUDE_DIRS ${FFMPEG_avdevice_INCLUDE_DIRS} ${FFMPEG_avfilter_INCLUDE_DIRS} ${FFMPEG_avformat_INCLUDE_DIRS} ${FFMPEG_avcodec_INCLUDE_DIRS} ${FFMPEG_postproc_INCLUDE_DIRS} ${FFMPEG_swscale_INCLUDE_DIRS} ${FFMPEG_avutil_INCLUDE_DIRS})
SET(FFMPEG_LIBRARIES ${FFMPEG_avdevice_STATIC_LIBRARIES} ${FFMPEG_avfilter_STATIC_LIBRARIES} ${FFMPEG_avformat_STATIC_LIBRARIES} ${FFMPEG_avcodec_STATIC_LIBRARIES} ${FFMPEG_postproc_STATIC_LIBRARIES} ${FFMPEG_swscale_STATIC_LIBRARIES} ${FFMPEG_avutil_STATIC_LIBRARIES})
SET(FFMPEG_INCLUDE_DIR ${FFMPEG_avcodec_INCLUDE_DIRS})
SET(FFMPEG_FOUND ${FFMPEG_avcodec_FOUND})


set(ADDITIONAL_LIBS
	lzma lcms
)
