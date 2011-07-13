cmake_minimum_required(VERSION 2.8)

project(V3D)

# export V3D_SHADER_DIR=/data/cworkspace/slowmoGPU/GL/Shaders/

set(CMAKE_CXX_FLAGS
    ${CMAKE_CXX_FLAGS}
    -Wall
)


set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Config) 

# Should be changed later if required.
get_filename_component(CMAKE_INSTALL_PREFIX "${V3D_SOURCE_DIR}/../install/" ABSOLUTE)
message("Install prefix: ${CMAKE_INSTALL_PREFIX}")

# Include OpenGL
find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIR})

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

#Include GLUT (libglut3-dev)
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

#Include GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})

find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS})

set (V3D_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include (Config/v3d_macros.cmake)

set (V3D_INCLUDE_DIRS ${V3D_DIR}/.)

include_directories(${V3D_INCLUDE_DIRS} ${EXTRA_INC_DIRS})

add_definitions(-DDISABLE_REDEFINITIONS)
#--------------------------------------------------
enable_feature (V3DLIB_ENABLE_LIBJPEG)
enable_feature (V3DLIB_ENABLE_LIBPNG)
enable_feature_libraries (V3DLIB_ENABLE_LIBJPEG jpeg)
enable_feature_libraries (V3DLIB_ENABLE_LIBPNG png)

enable_feature (V3DLIB_ENABLE_OPENCV)
enable_feature (V3DLIB_ENABLE_GPGPU)
enable_conditional_feature (V3DLIB_GPGPU_ENABLE_CG V3DLIB_ENABLE_GPGPU)
enable_feature_libraries (V3DLIB_ENABLE_GPGPU GLEW GLU GL)
enable_feature_libraries (V3DLIB_GPGPU_ENABLE_CG Cg CgGL pthread)
enable_feature_libraries (V3DLIB_ENABLE_OPENCV highgui cvaux cv)
#--------------------------------------------------

#set (CMAKE_BUILD_TYPE Release)
set (CMAKE_BUILD_TYPE Debug)

include_directories(Config)

set (GL_SRC
    GL/v3d_gpubase.cpp
    GL/v3d_gpuflow.cpp
    GL/v3d_gpucolorflow.cpp
    GL/v3d_gpupyramid.cpp
)

set (ALL_SRC
    ${GL_SRC}
    Config/config.h
    Base/v3d_image.cpp
    Base/v3d_imageprocessing.h
    Base/v3d_exception.h
    Base/v3d_timer.h
    Base/v3d_serialization.h
    Base/v3d_utilities.h
)


set (SHADERS
    ./GL/Shaders/flow_cholesky_3x3.cg
    ./GL/Shaders/flow_warp_image_with_gain.cg
    ./GL/Shaders/pyramid_with_derivative_pass2.cg
    ./GL/Shaders/pyramid_pass2.cg
    ./GL/Shaders/pyramid_with_derivative_pass1v.cg
    ./GL/Shaders/pyramid_with_derivative_pass1h.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_p.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_CLG_update_u.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_DR_update_v.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_direct_update_uvq.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_DR_update_u.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_direct_update_p.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_vd.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_u_iter1.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_pb.cg
    ./GL/Shaders/OpticalFlow/tvl1_color_flow_direct_update_q.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_q_with_gain.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_b.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_CLG_PD_update_u.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_uv.cg
    ./GL/Shaders/OpticalFlow/tvl1_color_flow_new_update_uv.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_u_iterN.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_p.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_relaxed_compute_UV.cg
    ./GL/Shaders/OpticalFlow/tvl1_color_flow_QR_update_uv.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_uv_with_gain.cg
    ./GL/Shaders/OpticalFlow/tvl1_color_flow_direct_update_uv.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_new_update_p.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_DR_update_p.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_CLG_PD_update_q.cg
    ./GL/Shaders/OpticalFlow/tvl1_color_flow_new_update_q.cg
    ./GL/Shaders/flow_accumulate_tensor.cg
    ./GL/Shaders/pyramid_pass1v.cg
    ./GL/Shaders/pyramid_pass1h.cg
    ./GL/Shaders/flow_warp_image.cg
)

foreach (FILENAME ${SHADERS})
    set(FILE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
    set(FILE_DST "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}")
    configure_file(${FILE_SRC} ${FILE_DST} COPYONLY)
endforeach(FILENAME)
message(STATUS "Shader files copied")



add_library(V3D ${ALL_SRC})
add_subdirectory(Apps)
target_link_libraries(V3D ${OpenCV_LIBS})


