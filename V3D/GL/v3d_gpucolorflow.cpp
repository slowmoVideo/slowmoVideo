#if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "Base/v3d_utilities.h"
#include "v3d_gpucolorflow.h"

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
      static Cg_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new Cg_FragmentProgram("v3d_gpuflow::upsampleDisparities::upsampleShader");

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
   upsampleDisparities(unsigned uvSrcTex, unsigned pSrcTex, unsigned qSrcTex, float pScale,
                       RTT_Buffer& ubuffer, RTT_Buffer& pbuffer, RTT_Buffer& qbuffer)
   {
      static Cg_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new Cg_FragmentProgram("v3d_gpuflow::upsampleDisparities::upsampleShader");

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
   TVL1_ColorFlowEstimator_Direct::allocate(int W, int H)
   {
      TVL1_ColorFlowEstimatorBase::allocate(W, H);

      _shader_uv = new Cg_FragmentProgram("tvl1_color_flow_direct_update_uv");
      _shader_uv->setProgramFromFile("OpticalFlow/tvl1_color_flow_direct_update_uv.cg");
      _shader_uv->compile();

      _shader_q = new Cg_FragmentProgram("tvl1_color_flow_direct_update_q");
      _shader_q->setProgramFromFile("OpticalFlow/tvl1_color_flow_direct_update_q.cg");
      _shader_q->compile();

      _shader_p = new Cg_FragmentProgram("tvl1_flow_direct_update_p");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_direct_update_p.cg");
      _shader_p->compile();

      _uBuffer1Pyramid.resize(_nLevels);
      _uBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);
      _qBuffer1Pyramid.resize(_nLevels);
      _qBuffer2Pyramid.resize(_nLevels);

      char const * uvTexSpec = _uvBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
      char const * pTexSpec  = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";
      char const * qTexSpec  = _pBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";

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
         _qBuffer1Pyramid[level] = new RTT_Buffer(qTexSpec, "qbuffer1");
         _qBuffer1Pyramid[level]->allocate(w, h);
         _qBuffer2Pyramid[level] = new RTT_Buffer(qTexSpec, "qbuffer2");
         _qBuffer2Pyramid[level]->allocate(w, h);
      } // end for (level)
   } // end TVL1_ColorFlowEstimator_Direct::allocate()

   void
   TVL1_ColorFlowEstimator_Direct::deallocate()
   {
      TVL1_ColorFlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBuffer1Pyramid[level]->deallocate();
         _uBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();
         _qBuffer1Pyramid[level]->deallocate();
         _qBuffer2Pyramid[level]->deallocate();
      }
   } // end TVL1_ColorFlowEstimator_Direct::deallocate()

   void
   TVL1_ColorFlowEstimator_Direct::run(unsigned int I0_TexIDs[3], unsigned int I1_TexIDs[3])
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * ubuffer1 = _uBuffer1Pyramid[level];
         RTT_Buffer * ubuffer2 = _uBuffer2Pyramid[level];
         RTT_Buffer * pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * pbuffer2 = _pBuffer2Pyramid[level];
         RTT_Buffer * qbuffer1 = _qBuffer1Pyramid[level];
         RTT_Buffer * qbuffer2 = _qBuffer2Pyramid[level];

         float const lambda = _lambda;
         //float const lambda = _lambda * sqrtf(1 << level);

         if (level == _nLevels-1)
         {
            glClearColor(0, 0, 0, 0);
            ubuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            pbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            qbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
         }
         else
         {
            //cout << "upsampleDisparities" << endl;
            upsampleDisparities(_uBuffer2Pyramid[level+1]->textureID(),
                                _pBuffer2Pyramid[level+1]->textureID(),
                                _qBuffer2Pyramid[level+1]->textureID(),
                                1.0f, *ubuffer2, *pbuffer2, *qbuffer2);
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer_R = *_warpedBufferPyramids[0][level];
         RTT_Buffer& warpedBuffer_G = *_warpedBufferPyramids[1][level];
         RTT_Buffer& warpedBuffer_B = *_warpedBufferPyramids[2][level];

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         _shader_uv->parameter("timestep", _cfg._tau_primal);
         _shader_q->parameter("timestep", _cfg._tau_dual);
         _shader_q->parameter("lambda_q", _cfg._lambdaScale);
         _shader_p->parameter("timestep", _cfg._tau_dual);
         _shader_p->parameter("rcpLambda_p", (lambda / _cfg._lambdaScale));

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
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_p->disable();

               //ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(pbuffer1, pbuffer2);

               ubuffer1->activate();

               //ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);
               qbuffer2->enableTexture(GL_TEXTURE2_ARB);
               warpedBuffer_R.enableTexture(GL_TEXTURE3_ARB);
               warpedBuffer_G.enableTexture(GL_TEXTURE4_ARB);
               warpedBuffer_B.enableTexture(GL_TEXTURE5_ARB);

               _shader_uv->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_uv->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(ubuffer1, ubuffer2);

               qbuffer1->activate();

               ubuffer2->enableTexture(GL_TEXTURE0_ARB);

               _shader_q->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_q->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               qbuffer2->disableTexture(GL_TEXTURE2_ARB);
               warpedBuffer_R.disableTexture(GL_TEXTURE3_ARB);
               warpedBuffer_G.disableTexture(GL_TEXTURE4_ARB);
               warpedBuffer_B.disableTexture(GL_TEXTURE5_ARB);

               std::swap(qbuffer1, qbuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_ColorFlowEstimator_Direct::run()

//----------------------------------------------------------------------

   void
   TVL1_ColorFlowEstimator_New::allocate(int W, int H)
   {
      TVL1_ColorFlowEstimatorBase::allocate(W, H);

      _shader_uv = new Cg_FragmentProgram("tvl1_color_flow_new_update_uv");
      _shader_uv->setProgramFromFile("OpticalFlow/tvl1_color_flow_new_update_uv.cg");
      _shader_uv->compile();

      _shader_q = new Cg_FragmentProgram("tvl1_color_flow_new_update_q");
      _shader_q->setProgramFromFile("OpticalFlow/tvl1_color_flow_new_update_q.cg");
      _shader_q->compile();

      _shader_p = new Cg_FragmentProgram("tvl1_flow_new_update_p");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_new_update_p.cg");
      _shader_p->compile();

      _uBuffer1Pyramid.resize(_nLevels);
      _uBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);
      _qBuffer1Pyramid.resize(_nLevels);
      _qBuffer2Pyramid.resize(_nLevels);

      char const * uvTexSpec = _uvBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
      char const * pTexSpec  = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";
      char const * qTexSpec  = _pBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";

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
         _qBuffer1Pyramid[level] = new RTT_Buffer(qTexSpec, "qbuffer1");
         _qBuffer1Pyramid[level]->allocate(w, h);
         _qBuffer2Pyramid[level] = new RTT_Buffer(qTexSpec, "qbuffer2");
         _qBuffer2Pyramid[level]->allocate(w, h);
      } // end for (level)
   } // end TVL1_ColorFlowEstimator_New::allocate()

   void
   TVL1_ColorFlowEstimator_New::deallocate()
   {
      TVL1_ColorFlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBuffer1Pyramid[level]->deallocate();
         _uBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();
         _qBuffer1Pyramid[level]->deallocate();
         _qBuffer2Pyramid[level]->deallocate();
      }
   } // end TVL1_ColorFlowEstimator_New::deallocate()

   void
   TVL1_ColorFlowEstimator_New::run(unsigned int I0_TexIDs[3], unsigned int I1_TexIDs[3])
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * ubuffer1 = _uBuffer1Pyramid[level];
         RTT_Buffer * ubuffer2 = _uBuffer2Pyramid[level];
         RTT_Buffer * pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * pbuffer2 = _pBuffer2Pyramid[level];
         RTT_Buffer * qbuffer1 = _qBuffer1Pyramid[level];
         RTT_Buffer * qbuffer2 = _qBuffer2Pyramid[level];

         float const lambda = _lambda;
         //float const lambda = _lambda / sqrtf(1 << level);

         if (level == _nLevels-1)
         {
            glClearColor(0, 0, 0, 0);
            ubuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            pbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            qbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
         }
         else
         {
            upsampleDisparities(_uBuffer2Pyramid[level+1]->textureID(),
                                _pBuffer2Pyramid[level+1]->textureID(),
                                _qBuffer2Pyramid[level+1]->textureID(),
                                1.0f, *ubuffer2, *pbuffer2, *qbuffer2);
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer_R = *_warpedBufferPyramids[0][level];
         RTT_Buffer& warpedBuffer_G = *_warpedBufferPyramids[1][level];
         RTT_Buffer& warpedBuffer_B = *_warpedBufferPyramids[2][level];

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         _shader_uv->parameter("timestep", _cfg._tau_primal);
         _shader_q->parameter("timestep", _cfg._tau_dual);
         _shader_q->parameter("lambda_q", _cfg._lambdaScale);
         _shader_p->parameter("timestep", _cfg._tau_dual);
         _shader_p->parameter("rcpLambda_p", (lambda / _cfg._lambdaScale));

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
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_p->disable();

               //ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(pbuffer1, pbuffer2);

               ubuffer1->activate();

               //ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);
               qbuffer2->enableTexture(GL_TEXTURE2_ARB);
               warpedBuffer_R.enableTexture(GL_TEXTURE3_ARB);
               warpedBuffer_G.enableTexture(GL_TEXTURE4_ARB);
               warpedBuffer_B.enableTexture(GL_TEXTURE5_ARB);

               _shader_uv->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_uv->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(ubuffer1, ubuffer2);

               qbuffer1->activate();

               ubuffer2->enableTexture(GL_TEXTURE0_ARB);

               _shader_q->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_q->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               qbuffer2->disableTexture(GL_TEXTURE2_ARB);
               warpedBuffer_R.disableTexture(GL_TEXTURE3_ARB);
               warpedBuffer_G.disableTexture(GL_TEXTURE4_ARB);
               warpedBuffer_B.disableTexture(GL_TEXTURE5_ARB);

               std::swap(qbuffer1, qbuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_ColorFlowEstimator_New::run()

//----------------------------------------------------------------------

   void
   TVL1_ColorFlowEstimator_QR::allocate(int W, int H)
   {
      TVL1_ColorFlowEstimatorBase::allocate(W, H);

      _shader_uv = new Cg_FragmentProgram("tvl1_color_flow_new_update_uv");
      _shader_uv->setProgramFromFile("OpticalFlow/tvl1_color_flow_QR_update_uv.cg");
      _shader_uv->compile();

      _shader_p = new Cg_FragmentProgram("tvl1_flow_relaxed_update_p");
      //_shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_update_p.cg");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_new_update_p.cg");
      _shader_p->compile();

      _uBuffer1Pyramid.resize(_nLevels);
      _uBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);

      char const * uvTexSpec = _uvBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
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

#endif
