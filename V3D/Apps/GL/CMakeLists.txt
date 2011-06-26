include_directories(${V3D_INCLUDE_DIRS} ${EXTRA_INC_DIRS})
link_directories(${V3D_DIR} ${EXTRA_LIB_DIRS})
link_libraries (V3D ${EXTRA_LIBRARIES} glut)

include_directories(${CMAKE_INSTALL_PREFIX}/include)
set(SVFLOW_PATHS "${CMAKE_INSTALL_PREFIX}/lib" /usr/local/lib)
find_library(SVFLOW_LIB
    NAMES sVflow sVflow.a libsVflow libsVflow.a
    PATHS ${SVFLOW_PATHS}
)
if(SVFLOW_LIB)
    message(STATUS "Flow library: ${SVFLOW_LIB}")

    if(V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)
      add_v3d_executable(tvl1_flow tvl1_flow.cpp)
      target_link_libraries(tvl1_flow ${SVFLOW_LIB})
    endif (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)

    if(V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)
      add_v3d_executable(flowBuilder flowBuilder.cpp)
      target_link_libraries(flowBuilder ${SVFLOW_LIB})
    endif (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)

#    if(V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)
#      add_v3d_executable(flowBuilderSDL flowBuilderSDL.cpp)
#      target_link_libraries(flowBuilderSDL ${SVFLOW_LIB} ${SDL_LIBRARY})
#    endif (V3DLIB_ENABLE_GPGPU AND V3DLIB_GPGPU_ENABLE_CG)

    install(TARGETS flowBuilder tvl1_flow DESTINATION bin)

else(SVFLOW_LIB)
    message(SEND_ERROR "slowmoVideo flow I/O library not found! Searched in: ${SVFLOW_PATHS}")
    message(STATUS "Cannot compile tvl1_flow.")
    message(STATUS "Cannot compile flowBuilder.")
endif(SVFLOW_LIB)


