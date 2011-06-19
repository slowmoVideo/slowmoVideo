set(SRCS_PROJ
  flow_sV.cpp
  project_sV.cpp
  tag_sV.cpp
  node_sV.cpp
  nodelist_sV.cpp
  renderTask_sV.cpp
  abstractProjectRW_sV.h
  xmlProjectRW_sV.cpp
  abstractFrameSource_sV.cpp
  imagesFrameSource_sV.cpp
  videoFrameSource_sV.cpp
  emptyFrameSource_sV.cpp
  abstractRenderTarget_sV.cpp
  imagesRenderTarget_sV.cpp
)

set(SRCS_MOC
  flow_sV.h
  project_sV.h
  renderTask_sV.h
  abstractFrameSource_sV.h
  imagesFrameSource_sV.h
  videoFrameSource_sV.h
  emptyFrameSource_sV.h
)

qt4_wrap_cpp(MOC_OUT ${SRCS_MOC})

add_library(sVproj STATIC ${SRCS_PROJ} ${MOC_OUT})
target_link_libraries(sVproj sV sVinfo sVflow ${QT_LIBRARIES})
