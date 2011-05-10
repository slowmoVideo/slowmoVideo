
set(SRCS
  test.cpp
)

add_executable(Test ${SRCS} ${MOC_OUT})
target_link_libraries(Test sVproj ${QT_LIBRARIES})

install(TARGETS Test DESTINATION bin)
