#ifndef CONFIG_H
#define CONFIG_H

/**

  This header file is for directly including the shader files in the binary
  which makes it easier to ship an exectuable.

  The #defines are only for QtCreator which does not support the CMake add_definitions yet.

  -- Simon A. Eugster
  */

#include <string>

#define V3DLIB_ENABLE_LIBJPEG
#define V3DLIB_ENABLE_LIBPNG
#define V3DLIB_ENABLE_GPGPU
#define V3DLIB_ENABLE_OPENCV
#define V3DLIB_GPGPU_ENABLE_CG

#define INCLUDE_SOURCE

#ifdef INCLUDE_SOURCE
#define STRINGIFY(A) #A

namespace GlShaderStrings {
#include "../GL/Shaders/flow_accumulate_tensor.cg"
#include "../GL/Shaders/flow_cholesky_3x3.cg"
#include "../GL/Shaders/flow_warp_image.cg"
#include "../GL/Shaders/flow_warp_image_with_gain.cg"
#include "../GL/Shaders/pyramid_pass1h.cg"
#include "../GL/Shaders/pyramid_pass1v.cg"
#include "../GL/Shaders/pyramid_pass2.cg"
#include "../GL/Shaders/pyramid_with_derivative_pass1h.cg"
#include "../GL/Shaders/pyramid_with_derivative_pass1v.cg"
#include "../GL/Shaders/pyramid_with_derivative_pass2.cg"

#include "../GL/Shaders/OpticalFlow/tvl1_color_flow_QR_update_uv.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_color_flow_direct_update_q.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_color_flow_direct_update_uv.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_color_flow_new_update_q.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_color_flow_new_update_uv.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_CLG_PD_update_q.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_CLG_PD_update_u.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_CLG_update_u.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_DR_update_p.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_DR_update_u.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_DR_update_v.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_b.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_p.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_pb.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_u_iter1.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_u_iterN.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_bregman_update_vd.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_direct_update_p.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_direct_update_uvq.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_new_update_p.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_relaxed_compute_UV.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_p.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_q_with_gain.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_uv.cg"
#include "../GL/Shaders/OpticalFlow/tvl1_flow_relaxed_update_uv_with_gain.cg"
}

#endif

#endif // CONFIG_H
