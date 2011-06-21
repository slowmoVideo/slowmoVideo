
# Building Qt+UI apps:
# http://www.qtcentre.org/wiki/index.php?title=Compiling_Qt4_apps_with_CMake
# http://www.cmake.org/pipermail/cmake/2008-September/023908.html


set(SRCS
  main.cpp
  mainwindow.cpp
  canvas.cpp
  inputMonitor.cpp
  newprojectdialog.cpp
  preferencesDialog.cpp
  progressDialog.cpp
  renderingDialog.cpp
  tagAddDialog.cpp
)

set(SRCS_UI
  mainwindow.ui
  canvas.ui
  inputMonitor.ui
  newprojectdialog.ui
  preferencesDialog.ui
  progressDialog.ui
  renderingDialog.ui
  tagAddDialog.ui
)

set(SRCS_MOC
  mainwindow.h
  canvas.h
  inputMonitor.h
  newprojectdialog.h
  preferencesDialog.h
  progressDialog.h
  renderingDialog.h
  tagAddDialog.h
)

set(RES
  #res/iconAdd.png
  res/iconSel.png
  res/iconMov.png
)
copy_files("${RES}")

# Generate header files from the .ui files
qt4_wrap_ui(UI_H_OUT ${SRCS_UI})
qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

# Include the generated header files
include_directories(${CMAKE_BINARY_DIR}/slowmoUI)

add_executable(slowmoUI ${SRCS} ${MOC_OUT} ${UI_H_OUT})
target_link_libraries(slowmoUI sV sVinfo sVproj ${QT_LIBRARIES})

install(TARGETS slowmoUI DESTINATION bin)
install(FILES ${RES} DESTINATION res PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
