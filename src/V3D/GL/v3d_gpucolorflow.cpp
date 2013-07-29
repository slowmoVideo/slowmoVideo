#include "config.h"

#include "Base/v3d_utilities.h"
#include "v3d_gpucolorflow.h"
#include "glsl_shaders.h"

#include <iostream>
#include <cmath>
#include <GL/glew.h>

using namespace std;
using namespace V3D_GPU;

namespace
{
   const std::string SOURCE(                                                                                                                                          
      "#version 330\n"
      "\n"                                                                                                                                                           
      "uniform sampler2D src_tex;\n"                                                                                                               
      "\n"                                                                                                                                                           
      "in vec4 gl_TexCoord[4];\n"
      "\n"
      "out vec4 my_FragColor;"
      "\n"                                                                                                                                                           
      "void main(void)\n"                                                                                                                                            
      "{\n"
      "   my_FragColor = gl_TexCoord[3] * texture2D(src_tex, gl_TexCoord[0].st);\n"                                                                                                           
      "}\n");
   
   void
   upsampleDisparities(unsigned uvSrcTex, unsigned pSrcTex, float pScale,
                       RTT_Buffer& ubuffer, RTT_Buffer& pbuffer)
   {
      static GLSL_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new GLSL_FragmentProgram("v3d_gpuflow::upsampleDisparities::upsampleShader");

         char const * source = SOURCE.c_str();
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
   upsampleDisparities(unsigned uvSrcTex, unsigned pSrcTex, unsigned qSrcTex, float pScale,
                       RTT_Buffer& ubuffer, RTT_Buffer& pbuffer, RTT_Buffer& qbuffer)
   {
      static GLSL_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new GLSL_FragmentProgram("v3d_gpuflow::upsampleDisparities::upsampleShader");

         char const * source = SOURCE.c_str();
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

      qbuffer.activate();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, qSrcTex);
      glEnable(GL_TEXTURE_2D);
      //upsampleShader->enable();
      // Provide uniform paramter via texcoord to avoid recompilation of shaders
      glMultiTexCoord4f(GL_TEXTURE3_ARB, pScale, pScale, pScale, pScale);
      renderNormalizedQuad();
      upsampleShader->disable();
      glDisable(GL_TEXTURE_2D);
      checkGLErrorsHere0();
   } // upsampleDisparities()

} // end namespace

//----------------------------------------------------------------------

namespace V3D_GPU
{

   void
   TVL1_ColorFlowEstimatorBase::allocate(int W, int H)
   {
      _width = W;
      _height = H;

      char const * texSpec = _warpedBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";

      for (int ch = 0; ch < 3; ++ch)
      {
         _warpedBufferPyramids[ch].resize(_nLevels);
         for (int level = 0; level < _nLevels; ++level)
         {
            int const w = _width / (1 << level);
            int const h = _height / (1 << level);

            _warpedBufferPyramids[ch][level] = new RTT_Buffer(texSpec, "_warpedBufferPyramid[]");
            _warpedBufferPyramids[ch][level]->allocate(w, h);
         }
      } // end for (ch)
   } // end TVL1_ColorFlowEstimatorBase::allocate()

   void
   TVL1_ColorFlowEstimatorBase::deallocate()
   {
      for (int ch = 0; ch < 3; ++ch)
         for (int level = 0; level < _nLevels; ++level)
            _warpedBufferPyramids[ch][level]->deallocate();
   }

//----------------------------------------------------------------------

   void
   TVL1_ColorFlowEstimator_QR::allocate(int W, int H)
   {
      TVL1_ColorFlowEstimatorBase::allocate(W, H);

      _shader_uv = new GLSL_FragmentProgram("tvl1_color_flow_new_update_uv");
      _shader_p = new GLSL_FragmentProgram("tvl1_flow_relaxed_update_p");
      _shader_uv->setProgram(GLSL_Shaders::tvl1_color_flow_QR_update_uv.c_str());
      _shader_p->setProgram(GLSL_Shaders::tvl1_flow_new_update_p.c_str());
      _shader_uv->compile();
      _shader_p->compile();

      _uBuffer1Pyramid.resize(_nLevels);
      _uBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);

      char const * uvTexSpec = _uvBufferHighPrecision ? "rg=32f tex2D enableTextureRG" : "rg=16f tex2D enableTextureRG";
      char const * pTexSpec  = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";

      for (int level = 0; level < _nLevels; ++level)
      {
         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         _uBuffer1Pyramid[level] = new RTT_Buffer(uvTexSpec, "ubuffer1");
         _uBuffer1Pyramid[level]->allocate(w, h);
         _uBuffer2Pyramid[level] = new RTT_Buffer(uvTexSpec, "ubuffer2");
         _uBuffer2Pyramid[level]->allocate(w, h);
         _pBuffer1Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer1");
         _pBuffer1Pyramid[level]->allocate(w, h);
         _pBuffer2Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer2");
         _pBuffer2Pyramid[level]->allocate(w, h);
      } // end for (level)
   } // end TVL1_ColorFlowEstimator_QR::allocate()

   void
   TVL1_ColorFlowEstimator_QR::deallocate()
   {
      TVL1_ColorFlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBuffer1Pyramid[level]->deallocate();
         _uBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();
      }
   } // end TVL1_ColorFlowEstimator_QR::deallocate()

   void
   TVL1_ColorFlowEstimator_QR::run(unsigned int I0_TexIDs[3], unsigned int I1_TexIDs[3])
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * ubuffer1 = _uBuffer1Pyramid[level];
         RTT_Buffer * ubuffer2 = _uBuffer2Pyramid[level];
         RTT_Buffer * pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * pbuffer2 = _pBuffer2Pyramid[level];

         float const lambda = _lambda;

         if (level == _nLevels-1)
         {
            glClearColor(0, 0, 0, 0);
            ubuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            pbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
         }
         else
         {
            upsampleDisparities(_uBuffer2Pyramid[level+1]->textureID(),
                                _pBuffer2Pyramid[level+1]->textureID(),
                                1.0f, *ubuffer2, *pbuffer2);
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer_R = *_warpedBufferPyramids[0][level];
         RTT_Buffer& warpedBuffer_G = *_warpedBufferPyramids[1][level];
         RTT_Buffer& warpedBuffer_B = *_warpedBufferPyramids[2][level];

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         _shader_uv->parameter("lambda_theta", 3.0f * _cfg._theta * lambda);
         _shader_uv->parameter("theta", _cfg._theta);
         //_shader_p->parameter("timestep_over_theta", _cfg._tau / _cfg._theta);
         _shader_p->parameter("timestep", -_cfg._tau / _cfg._theta);
         _shader_p->parameter("rcpLambda_p", 1.0f);

         for (int iter = 0; iter < _nOuterIterations; ++iter)
         {
            warpImageWithFlowField(ubuffer2->textureID(), I0_TexIDs[0], I1_TexIDs[0], level, warpedBuffer_R);
            warpImageWithFlowField(ubuffer2->textureID(), I0_TexIDs[1], I1_TexIDs[1], level, warpedBuffer_G);
            warpImageWithFlowField(ubuffer2->textureID(), I0_TexIDs[2], I1_TexIDs[2], level, warpedBuffer_B);
            checkGLErrorsHere0();

            setupNormalizedProjection();

            for (int k = 0; k < _nInnerIterations /* * sqrtf(resizeFactor) */; ++k)
            {
               pbuffer1->activate();

               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);

               _shader_p->enable();
               _shader_p->bindTexture("uv_src", 0);
               _shader_p->bindTexture("p_uv_src", 1);
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_p->disable();

               //ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(pbuffer1, pbuffer2);

               ubuffer1->activate();

               //ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);
               warpedBuffer_R.enableTexture(GL_TEXTURE2_ARB);
               warpedBuffer_G.enableTexture(GL_TEXTURE3_ARB);
               warpedBuffer_B.enableTexture(GL_TEXTURE4_ARB);


               _shader_uv->enable();
               _shader_uv->bindTexture("uv_src", 0);
               _shader_uv->bindTexture("p_uv_src", 1);
               _shader_uv->bindTexture("warped_R_tex", 2);
               _shader_uv->bindTexture("warped_G_tex", 3);
               _shader_uv->bindTexture("warped_B_tex", 4);
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_uv->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);
               warpedBuffer_R.disableTexture(GL_TEXTURE2_ARB);
               warpedBuffer_G.disableTexture(GL_TEXTURE3_ARB);
               warpedBuffer_B.disableTexture(GL_TEXTURE4_ARB);

               std::swap(ubuffer1, ubuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_ColorFlowEstimator_QR::run()

} // end namespace V3D_GP
