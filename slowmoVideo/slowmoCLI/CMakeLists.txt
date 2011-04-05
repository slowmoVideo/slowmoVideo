

set(SOURCES
  main.cpp
)

add_executable(slowmoVideo ${SOURCES})
target_link_libraries(slowmoVideo sV ${QT_LIBRARIES})

install(TARGETS slowmoVideo DESTINATION bin)