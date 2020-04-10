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
  exportVideoRenderTarget.cpp
  work_flow.cpp
)

if (OLD_FFMPEG)
set(SRCS_PROJ
	${SRCS_PROJ}
  new_videoRenderTarget.cpp
)
endif()

include_directories(${FFMPEG_INCLUDE_PATHS})
add_library(sVproj STATIC ${SRCS_PROJ})
qt5_use_modules(sVproj Core)
qt5_use_modules(sVproj Gui)
qt5_use_modules(sVproj Xml)
qt5_use_modules(sVproj Script)
target_link_libraries(sVproj sV sVinfo sVflow sVencode ${EXTERNAL_LIBS})
