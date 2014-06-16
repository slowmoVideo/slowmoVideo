set(SRCS_PROJ
  project_sV.cpp
  projectPreferences_sV.cpp
  renderPreferences_sV.cpp
  tag_sV.cpp
  node_sV.cpp
  nodeList_sV.cpp
  segment_sV.cpp
  segmentList_sV.cpp
  nodeHandle_sV.cpp
  renderTask_sV.cpp
  xmlProjectRW_sV.cpp
  abstractFrameSource_sV.cpp
  imagesFrameSource_sV.cpp
  videoFrameSource_sV.cpp
  emptyFrameSource_sV.cpp
  abstractRenderTarget_sV.cpp
  imagesRenderTarget_sV.cpp
  videoRenderTarget_sV.cpp
  abstractFlowSource_sV.cpp
  flowSourceOpenCV_sV.cpp
  flowSourceV3D_sV.cpp
  interpolator_sV.cpp
  shutterFunction_sV.cpp
  shutterFunctionList_sV.cpp
  motionBlur_sV.cpp
  canvasObject_sV.h
  new_videoRenderTarget.cpp
  work_flow.cpp
)

set(SRCS_MOC
  project_sV.h
  renderTask_sV.h
  abstractFrameSource_sV.h
  imagesFrameSource_sV.h
  videoFrameSource_sV.h
  emptyFrameSource_sV.h
  work_flow.h
)

qt_wrap_cpp(MOC_OUT ${SRCS_MOC})

include_directories(${FFMPEG_INCLUDE_PATHS})
add_library(sVproj STATIC ${SRCS_PROJ} ${MOC_OUT})
qt_use_modules(sVproj Core)
qt_use_modules(sVproj Gui)
qt_use_modules(sVproj Xml)
target_link_libraries(sVproj sV sVinfo sVflow sVencode ${EXTERNAL_LIBS})
