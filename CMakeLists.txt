project(V3D)

# export V3D_SHADER_DIR=/data/cworkspace/slowmoGPU/GL/Shaders/

cmake_minimum_required(VERSION 2.8)

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Config) 

# Include OpenGL
find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIR})

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

set (GL_SRC
    GL/v3d_gpubase.cpp
    GL/v3d_gpupyramid.cpp
    GL/v3d_gpuklt.cpp
    GL/v3d_gpuundistort.cpp
    GL/v3d_gpuflow.cpp
)

set (ALL_SRC
  Base/v3d_image.cpp
  ${GL_SRC}
)


set (SHADERS
    ./GL/Shaders/klt_detector_nonmax.cg
    ./GL/Shaders/klt_detector_traverse_histpyr.cg
    ./GL/Shaders/pyramid_with_derivative_pass2.cg
    ./GL/Shaders/klt_detector_pass1.cg
    ./GL/Shaders/klt_detector_pass2.cg
    ./GL/Shaders/klt_tracker_with_gain.cg
    ./GL/Shaders/klt_detector_discriminator.cg
    ./GL/Shaders/klt_detector_build_histpyr.cg
    ./GL/Shaders/klt_tracker.cg
    ./GL/Shaders/pyramid_pass2.cg
    ./GL/Shaders/undistort_parametric.cg
    ./GL/Shaders/pyramid_with_derivative_pass1v.cg
    ./GL/Shaders/pyramid_with_derivative_pass1h.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_p.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_uv.cg
    ./GL/Shaders/OpticalFlow/tvl1_flow_relaxed_compute_UV.cg
    ./GL/Shaders/pyramid_pass1v.cg
    ./GL/Shaders/pyramid_pass1h.cg
    ./GL/Shaders/flow_warp_image.cg
)

message("-- Copying files")
macro(copy_files FILELIST)
  foreach (FILENAME ${FILELIST})
    set(FILE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
    set(FILE_DST "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}")
    message("   Copying " ${FILENAME} " to " ${FILE_DST})

    configure_file(${FILE_SRC} ${FILE_DST} COPYONLY)
  endforeach(FILENAME)
endmacro(copy_files)
copy_files("${SHADERS}")



add_library(V3D ${ALL_SRC})
add_subdirectory(Apps)
target_link_libraries(V3D ${OpenCV_LIBS})


message(-- "Found? ${OpenCV_LIBS}.")
message(-- " OpenCV Dirs: ${OpenCV_INCLUDE_DIRS}")
