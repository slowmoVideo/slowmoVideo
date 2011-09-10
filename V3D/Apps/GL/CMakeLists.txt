include_directories(${V3D_INCLUDE_DIRS} ${EXTRA_INC_DIRS})
link_directories(${V3D_DIR} ${EXTRA_LIB_DIRS})
# (WIN) Check: Really more than extra libs required?
link_libraries (V3D ${EXTRA_LIBRARIES} ${GLUT_LIBRARIES} ${JPEG_LIBRARIES} ${GLEW_LIBRARIES} ${CG_LIBRARY} ${OPENGL_LIBRARIES})


message(STATUS "Flow library: ${SVFLOW_LIB}")

#if(V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)
#  add_v3d_executable(tvl1_flow tvl1_flow.cpp)
#  target_link_libraries(tvl1_flow ${SVFLOW_LIB})
#endif (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)

if(V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)
  add_v3d_executable(flowBuilder flowBuilder.cpp)
  target_link_libraries(flowBuilder ${SVFLOW_LIB})
endif (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)

#    if(V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)
#      add_v3d_executable(flowBuilderSDL flowBuilderSDL.cpp)
#      target_link_libraries(flowBuilderSDL ${SVFLOW_LIB} ${SDL_LIBRARY})
#    endif (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)

#install(TARGETS tvl1_flow DESTINATION bin)
install(TARGETS flowBuilder DESTINATION bin)

