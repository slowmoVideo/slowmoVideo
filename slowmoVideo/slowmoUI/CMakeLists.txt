
# Building Qt+UI apps:
# http://www.qtcentre.org/wiki/index.php?title=Compiling_Qt4_apps_with_CMake
# http://www.cmake.org/pipermail/cmake/2008-September/023908.html


set(SRCS
  main.cpp
  mainwindow.cpp
  canvas.cpp
)

set(SRCS_UI
  mainwindow.ui
  canvas.ui
)

set(SRCS_MOC
  mainwindow.h
  canvas.h
)

# Generate header files from the .ui files
qt4_wrap_ui(UI_H ${SRCS_UI})
qt4_wrap_cpp(MOC_SRC ${SRCS_MOC})

# Include the generated header files
include_directories(${CMAKE_BINARY_DIR}/slowmoUI)

add_executable(slowmoUI ${SRCS} ${MOC_SRC} ${UI_H})
target_link_libraries(slowmoUI ${QT_LIBRARIES})

install(TARGETS slowmoUI DESTINATION bin)
