include_directories(${V3D_INCLUDE_DIRS} ${EXTRA_INC_DIRS} ${slowmoVideo_SOURCE_DIR}/slowmoVideo/lib)
link_directories(${V3D_DIR} ${EXTRA_LIB_DIRS})
link_libraries (V3D ${EXTRA_LIBRARIES})

if(V3DLIB_ENABLE_GPGPU)
  add_v3d_executable(
    slowmoFlowBuilder
    flowBuilder.cpp
    ${slowmoVideo_SOURCE_DIR}/slowmoVideo/lib/flowField_sV.cpp
    ${slowmoVideo_SOURCE_DIR}/slowmoVideo/lib/flowRW_sV.cpp
  )
endif (V3DLIB_ENABLE_GPGPU)

install(TARGETS slowmoFlowBuilder DESTINATION bin)

