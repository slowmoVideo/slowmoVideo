
include_directories(${slowmoVideo_SOURCE_DIR})

set(SRCS
    visualizeFlow.cpp
)

add_executable(visualizeFlow ${SRCS})
target_link_libraries(visualizeFlow sVvis ${QT_LIBRARIES} ${ADDITIONAL_LIBS})

install(TARGETS visualizeFlow DESTINATION bin)
