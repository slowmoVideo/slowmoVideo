
include_directories(${slowmoVideo_SOURCE_DIR})

set(SRCS
    visualizeFlow.cpp
)

add_executable(slowmoVisualizeFlow ${SRCS})
target_link_libraries(slowmoVisualizeFlow sVvis ${EXTERNAL_LIBS})

install(TARGETS slowmoVisualizeFlow DESTINATION bin)
