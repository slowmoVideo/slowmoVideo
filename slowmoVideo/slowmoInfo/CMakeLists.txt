
add_executable(slowmoInfo slowmoInfo.cpp)
target_link_libraries(slowmoInfo sV ${QT_LIBRARIES} ${ADDITIONAL_LIBS})

install(TARGETS slowmoInfo DESTINATION "bin")
