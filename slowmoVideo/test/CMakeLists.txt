
add_executable(Test test.cpp ../project/project_sV.cpp)
target_link_libraries(Test sV sVI ${QT_LIBRARIES})

install(TARGETS Test DESTINATION bin)
