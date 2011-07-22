set(SRCS_PROJ
  project_sV.cpp
  projectPreferences_sV.cpp
  tag_sV.cpp
  node_sV.cpp
  nodelist_sV.cpp
  segment_sV.cpp
  nodeHandle_sV.cpp
  renderTask_sV.cpp
  abstractProjectRW_sV.h
  xmlProjectRW_sV.cpp
  abstractFrameSource_sV.cpp
  imagesFrameSource_sV.cpp
  videoFrameSource_sV.cpp
  emptyFrameSource_sV.cpp
  abstractRenderTarget_sV.cpp
  imagesRenderTarget_sV.cpp
  videoRenderTarget_sV.cpp
  abstractFlowSource_sV.cpp
  v3dFlowSource_sV.cpp
  interpolator_sV.cpp
  shutterFunction_sV.cpp
  shutterFunctionList_sV.cpp
  motionBlur_sV.cpp
  canvasObject_sV.h
)

set(SRCS_MOC
  project_sV.h
  renderTask_sV.h
  abstractFrameSource_sV.h
  imagesFrameSource_sV.h
  videoFrameSource_sV.h
  emptyFrameSource_sV.h
)

qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

add_library(sVproj STATIC ${SRCS_PROJ} ${MOC_OUT})
target_link_libraries(sVproj sV sVinfo sVflow sVencode ${QT_LIBRARIES})
