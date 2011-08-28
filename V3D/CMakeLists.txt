cmake_minimum_required(VERSION 2.8)

project(V3D)

#set (CMAKE_BUILD_TYPE Release)
set (CMAKE_BUILD_TYPE Debug)



# Thanks to David Cole: http://www.mail-archive.com/cmake@cmake.org/msg07872.html
get_filename_component(SV_INST_DIR "${V3D_SOURCE_DIR}/../install/" ABSOLUTE)
message(STATUS "Directory is ${SV_INST_DIR}")
set(INSTALL_PREFIX ${SV_INST_DIR} CACHE PATH "Installation prefix (CMAKE_INSTALL_PREFIX will NOT work)")
set(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX} CACHE INTERNAL "Install prefix" FORCE)


set(CMAKE_CXX_FLAGS
    ${CMAKE_CXX_FLAGS}
    -Wall
)


set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Config) 
find_package(OpenGL)
find_package(GLUT)
find_package(GLEW)
find_package(SDL)
find_package(Cg)
find_package(OpenCV)

include_directories(${CMAKE_INSTALL_PREFIX}/include)
set(SVFLOW_PATHS "${CMAKE_INSTALL_PREFIX}/lib" /usr/local/lib)
find_library(SVFLOW_LIB
    NAMES sVflow sVflow.a libsVflow libsVflow.a
    PATHS ${SVFLOW_PATHS}
)

set(BUILD_FLOW_BUILDER "NO")
if(SVFLOW_LIB AND OPENGL_FOUND AND SDL_FOUND AND GLUT_FOUND AND GLEW_FOUND AND CG_FOUND AND OpenCV_LIBS)
set(BUILD_FLOW_BUILDER "YES")
endif(SVFLOW_LIB AND OPENGL_FOUND AND SDL_FOUND AND GLUT_FOUND AND GLEW_FOUND AND CG_FOUND AND OpenCV_LIBS)

set(INCLUDE_SOURCE "YES")
if(DISABLE_INCLUDE_SOURCE)
set(INCLUDE_SOURCE "NO")
add_definitions(-DDISABLE_INCLUDE_SOURCE)
endif(DISABLE_INCLUDE_SOURCE)




message("==================================================")
if(NOT SVFLOW_LIB)
message("* sVflow library not found. Did you make --install it?")
message("  (Searched in ${SVFLOW_PATHS} for the sVflow library.)")
else(NOT SVFLOW_LIB)
message("* (ok) sVflow found")
endif(NOT SVFLOW_LIB)

if(NOT OPENGL_FOUND)
message("* OpenGL could not be found.")
else(NOT OPENGL_FOUND)
message("* (ok) OpenGL found in ${OPENGL_INCLUDE_DIR}")
endif(NOT OPENGL_FOUND)

if(NOT GLUT_FOUND)
message("* GLUT could not be found.")
else(NOT GLUT_FOUND)
message("* (ok) GLUT found in ${GLUT_INCLUDE_DIR}")
endif(NOT GLUT_FOUND)

if(NOT GLEW_FOUND)
message("* GLEW could not be found.")
else(NOT GLEW_FOUND)
message("* (ok) GLEW found at ${GLEW_INCLUDE_DIR}")
endif(NOT GLEW_FOUND)

if(NOT SDL_FOUND)
message("* SDL could not be found.")
else(NOT SDL_FOUND)
message("* (ok) SDL found in ${SDL_INCLUDE_DIR}")
endif(NOT SDL_FOUND)

if(NOT OpenCV_LIBS)
message("* OpenCV could not be found.")
else(NOT OpenCV_LIBS)
message("* (ok) OpenCV found in ${OpenCV_LIB_DIR}")
endif(NOT OpenCV_LIBS)

if(NOT CG_FOUND)
message("* Cg toolkit could not be found.")
message("  Download from: http://developer.nvidia.com/cg-toolkit-download")
else(NOT CG_FOUND)
message("* (ok) Cg found at ${CG_LIBRARY}")
endif(NOT CG_FOUND)


message("* Installation prefix: ${CMAKE_INSTALL_PREFIX}.")
message("  (Can be adjusted with the INSTALL_PREFIX variable (not CMAKE_INSTALL_PREFIX). \n  Default is ${SV_INST_DIR}.)")
message("* V3D will be built: ${BUILD_FLOW_BUILDER}")
message("* Shaders will be included in the binary: ${INCLUDE_SOURCE}")
if(INCLUDE_SOURCE)
message("  (Can be disabled with the cmake flag -DDISABLE_INCLUDE_SOURCE)")
endif(INCLUDE_SOURCE)
message("==================================================")

if(NOT BUILD_FLOW_BUILDER)
message(FATAL_ERROR "Cannot build V3D.")
endif(NOT BUILD_FLOW_BUILDER)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${SDL_INCLUDE_DIR})
include_directories(${GLUT_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
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
enable_feature_libraries (V3DLIB_ENABLE_OPENCV ${OpenCV_LIBS})
#--------------------------------------------------


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
