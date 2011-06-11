include_directories(${V3D_INCLUDE_DIRS} ${EXTRA_INC_DIRS})
link_directories(${V3D_DIR} ${EXTRA_LIB_DIRS})
link_libraries (V3D ${EXTRA_LIBRARIES} glut)

if (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)
  add_v3d_executable (tvl1_flow tvl1_flow.cpp flowRW.cpp)
endif (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)

if (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)
  add_v3d_executable (flowBuilder flowBuilder.cpp flowRW.cpp)
endif (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)

install(TARGETS flowBuilder tvl1_flow DESTINATION bin)
