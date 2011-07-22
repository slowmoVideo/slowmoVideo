
# Building Qt+UI apps:
# http://www.qtcentre.org/wiki/index.php?title=Compiling_Qt4_apps_with_CMake
# http://www.cmake.org/pipermail/cmake/2008-September/023908.html


include_directories(${slowmoVideo_SOURCE_DIR})

set(SRCS
  main.cpp
  mainwindow.cpp
  canvas.cpp
  inputMonitor.cpp
  dialogues/newProjectDialog.cpp
  dialogues/preferencesDialog.cpp
  dialogues/progressDialog.cpp
  dialogues/renderingDialog.cpp
  dialogues/shutterFunctionDialog.cpp
  dialogues/shutterFunctionFrame.cpp
  dialogues/tagAddDialog.cpp
  dialogues/aboutDialog.cpp
)

set(SRCS_UI
  mainwindow.ui
  canvas.ui
  inputMonitor.ui
  dialogues/newProjectDialog.ui
  dialogues/preferencesDialog.ui
  dialogues/progressDialog.ui
  dialogues/renderingDialog.ui
  dialogues/shutterFunctionDialog.ui
  dialogues/tagAddDialog.ui
  dialogues/aboutDialog.ui
)

set(SRCS_MOC
  mainwindow.h
  canvas.h
  inputMonitor.h
  dialogues/newProjectDialog.h
  dialogues/preferencesDialog.h
  dialogues/progressDialog.h
  dialogues/renderingDialog.h
  dialogues/shutterFunctionDialog.h
  dialogues/shutterFunctionFrame.h
  dialogues/tagAddDialog.h
  dialogues/aboutDialog.h
)

include_directories(dialogues)

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
target_link_libraries(slowmoUI sVproj ${QT_LIBRARIES})

install(TARGETS slowmoUI DESTINATION bin)
install(FILES ${RES} DESTINATION bin/res PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
