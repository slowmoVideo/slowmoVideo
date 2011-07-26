
include_directories(${slowmoVideo_SOURCE_DIR})

set(SRCS
    visualizeFlow.cpp
    flowErrors_sV.cpp
)

add_executable(visualizeFlow ${SRCS})
target_link_libraries(visualizeFlow sVflow ${QT_LIBRARIES})

install(TARGETS visualizeFlow DESTINATION bin)
