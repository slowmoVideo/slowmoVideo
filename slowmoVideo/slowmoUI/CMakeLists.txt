
# Building Qt+UI apps:
# http://www.qtcentre.org/wiki/index.php?title=Compiling_Qt4_apps_with_CMake
# http://www.cmake.org/pipermail/cmake/2008-September/023908.html


include_directories(${slowmoVideo_SOURCE_DIR})

set(SRCS
  main.cpp
  mainwindow.cpp
  canvas.cpp
  canvasTools.cpp
  frameMonitor.cpp
  renderPreview.cpp
  dialogues/newProjectDialog.cpp
  dialogues/preferencesDialog.cpp
  dialogues/projectPreferencesDialog.cpp
  dialogues/progressDialog.cpp
  dialogues/renderingDialog.cpp
  dialogues/shutterFunctionDialog.cpp
  dialogues/shutterFunctionFrame.cpp
  dialogues/flowExaminer.cpp
  dialogues/tagAddDialog.cpp
  dialogues/aboutDialog.cpp
)

set(SRCS_UI
  mainwindow.ui
  canvas.ui
  frameMonitor.ui
  renderPreview.ui
  dialogues/newProjectDialog.ui
  dialogues/preferencesDialog.ui
  dialogues/projectPreferencesDialog.ui
  dialogues/progressDialog.ui
  dialogues/renderingDialog.ui
  dialogues/shutterFunctionDialog.ui
  dialogues/flowExaminer.ui
  dialogues/tagAddDialog.ui
  dialogues/aboutDialog.ui
)

set(SRCS_MOC
  mainwindow.h
  canvas.h
  frameMonitor.h
  renderPreview.h
  dialogues/newProjectDialog.h
  dialogues/preferencesDialog.h
  dialogues/projectPreferencesDialog.h
  dialogues/progressDialog.h
  dialogues/renderingDialog.h
  dialogues/shutterFunctionDialog.h
  dialogues/shutterFunctionFrame.h
  dialogues/flowExaminer.h
  dialogues/tagAddDialog.h
  dialogues/aboutDialog.h
)


# Without these includes the promoted widgets fail to compile
# since the headers are not found. (For whatever reason.)
include_directories(dialogues)
include_directories(.)
include_directories(${slowmoVideo_SOURCE_DIR}/libgui)

# Embed images in the binary
set(SRC_RES resources.qrc)
qt4_add_resources(RES_OUT ${SRC_RES})

# Generate header files from the .ui files
qt4_wrap_ui(UI_H_OUT ${SRCS_UI})
qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

# Include the generated header files
include_directories(${CMAKE_BINARY_DIR}/slowmoUI)

add_executable(slowmoUI ${SRCS} ${MOC_OUT} ${UI_H_OUT} ${RES_OUT})
target_link_libraries(slowmoUI sVproj sVvis sVgui ${QT_LIBRARIES})

install(TARGETS slowmoUI DESTINATION bin)
