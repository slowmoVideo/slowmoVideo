
add_executable(visualizeFlow visualizeFlow.cpp)
target_link_libraries(visualizeFlow sVflow ${QT_LIBRARIES})

install(TARGETS visualizeFlow DESTINATION bin)
