
add_executable(visualizeFlow visualizeFlow.cpp)
target_link_libraries(visualizeFlow sVflowRW ${QT_LIBRARIES})

install(TARGETS visualizeFlow DESTINATION bin)
