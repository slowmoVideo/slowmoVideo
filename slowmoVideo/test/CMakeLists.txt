
set(SRCS
  test.cpp
  ../project/project_sV.cpp
)
set(SRCS_MOC
  ../project/project_sV.h
)
qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

add_executable(Test ${SRCS} ${MOC_OUT})
target_link_libraries(Test sV sVI ${QT_LIBRARIES})

install(TARGETS Test DESTINATION bin)
