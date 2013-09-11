
add_executable(slowmoInfo slowmoInfo.cpp)
target_link_libraries(slowmoInfo sV ${EXTERNAL_LIBS})

install(TARGETS slowmoInfo DESTINATION "bin")
