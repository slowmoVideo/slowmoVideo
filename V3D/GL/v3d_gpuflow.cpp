#include "config.h"

#include "Base/v3d_utilities.h"
#include "v3d_gpuflow.h"
#include "glsl_shaders.h"

#include <iostream>
#include <cmath>
#include <GL/glew.h>

using namespace std;
using namespace V3D_GPU;

namespace
{
   void
   upsampleDisparities(unsigned uvSrcTex, unsigned pSrcTex, float pScale,
                       RTT_Buffer& ubuffer, RTT_Buffer& pbuffer)
   {
      static GLSL_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new GLSL_FragmentProgram("v3d_gpuflow::upsampleDisparities::upsampleShader");

         char const * source =
            "void main(uniform sampler2D src_tex : TEXTURE0, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "                  float4 st3 : TEXCOORD3, \n"
            "              out float4 res_out : COLOR0) \n"
            "{ \n"
            "   res_out = st3 * tex2D(src_tex, st0); \n"
            "} \n";
         upsampleShader->setProgram(source);
         upsampleShader->compile();
         checkGLErrorsHere0();
      } // end if

      setupNormalizedProjection();
      ubuffer.activate();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, uvSrcTex);
      glEnable(GL_TEXTURE_2D);
      upsampleShader->enable();
      // Provide uniform paramter via texcoord to avoid recompilation of shaders
      glMultiTexCoord4f(GL_TEXTURE3_ARB, 2, 2, 1, 1);
      //glMultiTexCoord4f(GL_TEXTURE3_ARB, 0, 0, 0, 0);
      renderNormalizedQuad();
      //upsampleShader->disable();
      glDisable(GL_TEXTURE_2D);
      checkGLErrorsHere0();

      pbuffer.activate();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, pSrcTex);
      glEnable(GL_TEXTURE_2D);
      //upsampleShader->enable();
      // Provide uniform paramter via texcoord to avoid recompilation of shaders
      glMultiTexCoord4f(GL_TEXTURE3_ARB, pScale, pScale, pScale, pScale);
      renderNormalizedQuad();
      upsampleShader->disable();
      glDisable(GL_TEXTURE_2D);
      checkGLErrorsHere0();
   } // upsampleDisparities()

   void
   upsampleBuffer(unsigned srcTex, float scale, FrameBufferObject& dstFbo)
   {
      static GLSL_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new GLSL_FragmentProgram("v3d_gpuflow::upsampleBuffer::upsampleShader");

         char const * source =
            "void main(uniform sampler2D src_tex : TEXTURE0, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "                  float4 st3 : TEXCOORD3, \n"
            "              out float4 res_out : COLOR0) \n"
            "{ \n"
            "   res_out = st3 * tex2D(src_tex, st0); \n"
            "} \n";
         upsampleShader->setProgram(source);
         upsampleShader->compile();
         checkGLErrorsHere0();
      } // end if

      setupNormalizedProjection();
      dstFbo.activate();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, srcTex);
      glEnable(GL_TEXTURE_2D);
      upsampleShader->enable();
      // Provide uniform paramter via texcoord to avoid recompilation of shaders
      glMultiTexCoord4f(GL_TEXTURE3_ARB, scale, scale, scale, scale);
      renderNormalizedQuad();
      upsampleShader->disable();
      glDisable(GL_TEXTURE_2D);
      checkGLErrorsHere0();
   } // upsampleBuffer()

   void
   upsampleBuffers(unsigned src1Tex, unsigned src2Tex, float scale1, float scale2,
                   FrameBufferObject& dstFbo)
   {
      static GLSL_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new GLSL_FragmentProgram("v3d_gpuflow::upsampleBuffer::upsampleShader");

         char const * source =
            "void main(uniform sampler2D src1_tex : TEXTURE0, \n"
            "          uniform sampler2D src2_tex : TEXTURE1, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "                  float4 st3 : TEXCOORD3, \n"
            "                  float4 st4 : TEXCOORD4, \n"
            "              out float4 res1_out : COLOR0, \n"
            "              out float4 res2_out : COLOR1) \n"
            "{ \n"
            "   res1_out = st3 * tex2D(src1_tex, st0); \n"
            "   res1_out = st4 * tex2D(src2_tex, st0); \n"
            "} \n";
         upsampleShader->setProgram(source);
         upsampleShader->compile();
         checkGLErrorsHere0();
      } // end if

      setupNormalizedProjection();
      dstFbo.activate();

      GLenum const targetBuffers[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
      glDrawBuffersARB(2, targetBuffers);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, src1Tex);
      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, src2Tex);
      glEnable(GL_TEXTURE_2D);
      upsampleShader->enable();
      // Provide uniform paramter via texcoord to avoid recompilation of shaders
      // Texcoords 0-2 are assigned by renderNormalizedQuad().
      glMultiTexCoord4f(GL_TEXTURE3_ARB, scale1, scale1, scale1, scale1);
      glMultiTexCoord4f(GL_TEXTURE4_ARB, scale2, scale2, scale2, scale2);
      renderNormalizedQuad();
      upsampleShader->disable();
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE1);
      glDisable(GL_TEXTURE_2D);

      glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

      checkGLErrorsHere0();
   } // upsampleBuffers()

} // end namespace

//----------------------------------------------------------------------

namespace V3D_GPU
{

   void
   TVL1_FlowEstimatorBase::allocate(int W, int H)
   {
      _width = W;
      _height = H;

      char const * texSpec = _warpedBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";

      _warpedBufferPyramid.resize(_nLevels);
      for (int level = 0; level < _nLevels; ++level)
      {
         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         _warpedBufferPyramid[level] = new RTT_Buffer(texSpec, "_warpedBufferPyramid[]");
         _warpedBufferPyramid[level]->allocate(w, h);
      }
   } // end TVL1_FlowEstimatorBase::allocate()

   void
   TVL1_FlowEstimatorBase::deallocate()
   {
      for (int level = 0; level < _nLevels; ++level)
         _warpedBufferPyramid[level]->deallocate();
   }

//----------------------------------------------------------------------

   void
   warpImageWithFlowField(unsigned int uv_tex, unsigned int I0_tex,
                          unsigned int I1_tex, int level, RTT_Buffer& dest)
   {
      static GLSL_FragmentProgram * shader = 0;
      if (shader == 0)
      {
         shader = new GLSL_FragmentProgram("v3d_gpuflow::warpImageWithFlowField::shader");
         shader->setProgram(GLSL_Shaders::flow_warp_image.c_str());
         shader->compile();
         checkGLErrorsHere0();
      }

      int const w = dest.width();
      int const h = dest.height();

      dest.activate();

      setupNormalizedProjection();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, uv_tex);
      glEnable(GL_TEXTURE_2D);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, I0_tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, I1_tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
      glEnable(GL_TEXTURE_2D);

      // Provide uniform paramter via texcoord to avoid recompilation of shaders
      glMultiTexCoord3f(GL_TEXTURE3_ARB, 1.0f/w, 1.0f/h, 1 << level);
      shader->enable();
      shader->bindTexture("uv_src", 0);
      shader->bindTexture("I0_tex", 1);
      shader->bindTexture("I1_tex", 2);
      renderNormalizedQuad();
      shader->disable();

      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE1);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE2);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glDisable(GL_TEXTURE_2D);
   } // end warpImageWithFlowField()

} // end namespace V3D_GPU
