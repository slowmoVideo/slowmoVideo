
#include "config.h"

#if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "Base/v3d_utilities.h"
#include "v3d_gpuflow.h"

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
   upsampleBuffer(unsigned srcTex, float scale, FrameBufferObject& dstFbo)
   {
      static Cg_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new Cg_FragmentProgram("v3d_gpuflow::upsampleBuffer::upsampleShader");

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
      static Cg_FragmentProgram * upsampleShader = 0;

      if (upsampleShader == 0)
      {
         upsampleShader = new Cg_FragmentProgram("v3d_gpuflow::upsampleBuffer::upsampleShader");

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
   TVL1_FlowEstimator_Relaxed::allocate(int W, int H)
   {
      TVL1_FlowEstimatorBase::allocate(W, H);

      _shader_uv = new Cg_FragmentProgram("tvl1_flow_relaxed_update_uv");
      _shader_p = new Cg_FragmentProgram("tvl1_flow_relaxed_update_p");

#ifdef INCLUDE_SOURCE
      _shader_uv->setProgram(GlShaderStrings::tvl1_flow_relaxed_update_uv.c_str());
      _shader_p->setProgram(GlShaderStrings::tvl1_flow_relaxed_update_p.c_str());
#else
      _shader_uv->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_update_uv.cg");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_update_p.cg");
#endif

      _shader_uv->compile();
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
   } // end TVL1_FlowEstimator_Relaxed::allocate()

   void
   TVL1_FlowEstimator_Relaxed::deallocate()
   {
      TVL1_FlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBuffer1Pyramid[level]->deallocate();
         _uBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();
      }
   } // end TVL1_FlowEstimator_Relaxed::deallocate()

   void
   TVL1_FlowEstimator_Relaxed::run(unsigned int I0_TexID, unsigned int I1_TexID)
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * &ubuffer1 = _uBuffer1Pyramid[level];
         RTT_Buffer * &ubuffer2 = _uBuffer2Pyramid[level];
         RTT_Buffer * &pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * &pbuffer2 = _pBuffer2Pyramid[level];

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
            //cout << "upsampleDisparities" << endl;
            upsampleDisparities(_uBuffer2Pyramid[level+1]->textureID(), _pBuffer2Pyramid[level+1]->textureID(), 1.0f,
                                *ubuffer2, *pbuffer2);
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer = *_warpedBufferPyramid[level];

         float const theta  = _theta;
         //float const theta  = _theta / float(1 << level);
         float const lambda = _lambda;
         //float const lambda = _lambda * sqrtf(1 << level);

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         for (int iter = 0; iter < _nOuterIterations; ++iter)
         {
            warpImageWithFlowField(ubuffer2->textureID(), I0_TexID, I1_TexID, level, warpedBuffer);

            setupNormalizedProjection();

            _shader_uv->parameter("theta", theta);
            _shader_uv->parameter("lambda_theta", lambda*theta);
            _shader_p->parameter("timestep_over_theta", _tau/theta);

            checkGLErrorsHere0();

            for (int k = 0; k < _nInnerIterations /* * sqrtf(resizeFactor) */; ++k)
            {
               pbuffer1->activate();

               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);

               //shader_p->parameter("timestep_over_theta", timestep/theta);
               _shader_p->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_p->disable();

               //ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(pbuffer1, pbuffer2);

               ubuffer1->activate();

               //ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);
               warpedBuffer.enableTexture(GL_TEXTURE2_ARB);

               //_shader_uv->parameter("theta", theta);
               //_shader_uv->parameter("lambda_theta", lambda*theta);

               _shader_uv->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_uv->disable();

               warpedBuffer.disableTexture(GL_TEXTURE2_ARB);

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(ubuffer1, ubuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_FlowEstimator_Relaxed::run()

   void
   TVL1_FlowEstimator_Relaxed::computeAlternateFlow()
   {
      static Cg_FragmentProgram * shader = 0;

      if (!shader)
      {
         shader = new Cg_FragmentProgram("tvl1_flow_relaxed_compute_UV");
#ifdef INCLUDE_SOURCE
         shader->setProgram(GlShaderStrings::tvl1_flow_relaxed_compute_UV.c_str());
#else
         shader->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_compute_UV.cg");
#endif
         shader->compile();
      } // end if

      RTT_Buffer * &ubuffer1 = _uBuffer1Pyramid[_startLevel];
      RTT_Buffer * &ubuffer2 = _uBuffer2Pyramid[_startLevel];
      RTT_Buffer *  warpedBuffer = _warpedBufferPyramid[_startLevel];

      ubuffer1->activate();

      ubuffer2->enableTexture(GL_TEXTURE0_ARB);
      warpedBuffer->enableTexture(GL_TEXTURE2_ARB);

      shader->parameter("lambda_theta", _lambda*_theta);

      shader->enable();
      renderNormalizedQuad();
      shader->disable();

      warpedBuffer->disableTexture(GL_TEXTURE2_ARB);

      ubuffer2->disableTexture(GL_TEXTURE0_ARB);

      std::swap(ubuffer1, ubuffer2);
   } // end TVL1_FlowEstimator_Relaxed::computeAlternateFlow()

//----------------------------------------------------------------------

   void
   TVL1_FlowEstimator_WithGain::allocate(int W, int H)
   {
      TVL1_FlowEstimatorBase::allocate(W, H);

      _shader_uv = new Cg_FragmentProgram("tvl1_flow_relaxed_update_uv_with_gain");
      _shader_p = new Cg_FragmentProgram("tvl1_flow_relaxed_update_p");
      _shader_q = new Cg_FragmentProgram("tvl1_flow_relaxed_update_q_with_gain");

#ifdef INCLUDE_SOURCE
      _shader_uv->setProgram(GlShaderStrings::tvl1_flow_relaxed_update_uv_with_gain.c_str());
      _shader_p->setProgram(GlShaderStrings::tvl1_flow_relaxed_update_p.c_str());
      _shader_q->setProgram(GlShaderStrings::tvl1_flow_relaxed_update_q_with_gain.c_str());
#else
      _shader_uv->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_update_uv_with_gain.cg");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_update_p.cg");
      _shader_q->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_update_q_with_gain.cg");
#endif

      _shader_uv->compile();
      _shader_p->compile();
      _shader_q->compile();

      _uBuffer1Pyramid.resize(_nLevels);
      _uBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);
      _qBuffer1Pyramid.resize(_nLevels);
      _qBuffer2Pyramid.resize(_nLevels);

      _warpedPyramid.resize(_nLevels);
      _warped1Texs.resize(_nLevels);
      _warped2Texs.resize(_nLevels);

      char const * uvTexSpec  = _uvBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
      char const * pTexSpec   = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";
      char const * qTexSpec   = _pBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
      char const * warpedSpec = _warpedBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";

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

         //vector<unsigned char> pixels(w*h, 0);

         _warped1Texs[level].allocateID();
         _warped1Texs[level].reserve(w, h, TextureSpecification(warpedSpec));
         //_warped1Texs[level].overwriteWith(&pixels[0], 1);
         _warped2Texs[level].allocateID();
         _warped2Texs[level].reserve(w, h, TextureSpecification(warpedSpec));
         //_warped2Texs[level].overwriteWith(&pixels[0], 1);

         _warpedPyramid[level].allocate();
         _warpedPyramid[level].makeCurrent();
         _warpedPyramid[level].attachTexture2D(_warped1Texs[level], GL_COLOR_ATTACHMENT0_EXT);
         _warpedPyramid[level].attachTexture2D(_warped2Texs[level], GL_COLOR_ATTACHMENT1_EXT);
      } // end for (level)
   } // end TVL1_FlowEstimator_WithGain::allocate()

   void
   TVL1_FlowEstimator_WithGain::deallocate()
   {
      TVL1_FlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBuffer1Pyramid[level]->deallocate();
         _uBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();
         _qBuffer1Pyramid[level]->deallocate();
         _qBuffer2Pyramid[level]->deallocate();

         _warped1Texs[level].deallocateID();
         _warped2Texs[level].deallocateID();
         _warpedPyramid[level].deallocate();
      }
   } // end TVL1_FlowEstimator_WithGain::deallocate()

   void
   TVL1_FlowEstimator_WithGain::run(unsigned int I0_TexID, unsigned int I1_TexID)
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * &ubuffer1 = _uBuffer1Pyramid[level];
         RTT_Buffer * &ubuffer2 = _uBuffer2Pyramid[level];
         RTT_Buffer * &pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * &pbuffer2 = _pBuffer2Pyramid[level];
         RTT_Buffer * &qbuffer1 = _qBuffer1Pyramid[level];
         RTT_Buffer * &qbuffer2 = _qBuffer2Pyramid[level];

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
            upsampleDisparities(_uBuffer2Pyramid[level+1]->textureID(), _pBuffer2Pyramid[level+1]->textureID(), 1.0f,
                                *ubuffer2, *pbuffer2);
            upsampleBuffer(_qBuffer2Pyramid[level+1]->textureID(), 1.0f, qbuffer2->getFBO());
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         FrameBufferObject& warpedBuffer = _warpedPyramid[level];
         //RTT_Buffer& warpedBuffer = *_warpedBufferPyramid[level];

         float const theta  = _cfg._theta;
         float const lambda = _lambda;

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         for (int iter = 0; iter < _nOuterIterations; ++iter)
         {
            warpImageWithFlowFieldAndGain(ubuffer2->textureID(), I0_TexID, I1_TexID, level, warpedBuffer);
            //warpImageWithFlowField(ubuffer2->textureID(), I0_TexID, I1_TexID, level, warpedBuffer);

            setupNormalizedProjection();

            _shader_uv->parameter("lambda", lambda);
            _shader_uv->parameter("theta", theta, 1.0f / theta);
            _shader_uv->parameter("gamma", _cfg._gamma);
            _shader_uv->parameter("theta_delta", theta*_cfg._delta);

            _shader_p->parameter("timestep_over_theta", _cfg._tau/theta);
            _shader_q->parameter("timestep_over_theta_delta", _cfg._tau/(theta*_cfg._delta));

            checkGLErrorsHere0();

            for (int k = 0; k < _nInnerIterations /* * sqrtf(resizeFactor) */; ++k)
            {
               pbuffer1->activate();

               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);

               _shader_p->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_p->disable();

               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(pbuffer1, pbuffer2);

               qbuffer1->activate();

               qbuffer2->enableTexture(GL_TEXTURE1_ARB);

               _shader_q->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_q->disable();

               qbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(qbuffer1, qbuffer2);

               ubuffer1->activate();

               pbuffer2->enableTexture(GL_TEXTURE1_ARB);
               qbuffer2->enableTexture(GL_TEXTURE2_ARB);
               _warped1Texs[level].enable(GL_TEXTURE3_ARB);
               _warped2Texs[level].enable(GL_TEXTURE4_ARB);

               _shader_uv->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_uv->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);
               qbuffer2->disableTexture(GL_TEXTURE2_ARB);
               _warped1Texs[level].disable(GL_TEXTURE3_ARB);
               _warped2Texs[level].disable(GL_TEXTURE4_ARB);

               std::swap(ubuffer1, ubuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_FlowEstimator_WithGain::run()

//----------------------------------------------------------------------

   void
   TVL1_FlowEstimator_Direct::allocate(int W, int H)
   {
      TVL1_FlowEstimatorBase::allocate(W, H);

      _shader_uvq = new Cg_FragmentProgram("tvl1_flow_direct_update_uvq");
      _shader_p = new Cg_FragmentProgram("tvl1_flow_direct_update_p");

#ifdef INCLUDE_SOURCE
      _shader_uvq->setProgram(GlShaderStrings::tvl1_flow_direct_update_uvq.c_str());
      _shader_p->setProgram(GlShaderStrings::tvl1_flow_direct_update_p.c_str());
#else
      _shader_uvq->setProgramFromFile("OpticalFlow/tvl1_flow_direct_update_uvq.cg");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_direct_update_p.cg");
#endif

      _shader_uvq->compile();
      _shader_p->compile();

      _uqBuffer1Pyramid.resize(_nLevels);
      _uqBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);

      char const * uvTexSpec = _uvBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
      char const * pTexSpec  = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";

      for (int level = 0; level < _nLevels; ++level)
      {
         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         _uqBuffer1Pyramid[level] = new RTT_Buffer(uvTexSpec, "ubuffer1");
         _uqBuffer1Pyramid[level]->allocate(w, h);
         _uqBuffer2Pyramid[level] = new RTT_Buffer(uvTexSpec, "ubuffer2");
         _uqBuffer2Pyramid[level]->allocate(w, h);
         _pBuffer1Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer1");
         _pBuffer1Pyramid[level]->allocate(w, h);
         _pBuffer2Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer2");
         _pBuffer2Pyramid[level]->allocate(w, h);
      } // end for (level)
   } // end TVL1_FlowEstimator_Direct::allocate()

   void
   TVL1_FlowEstimator_Direct::deallocate()
   {
      TVL1_FlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uqBuffer1Pyramid[level]->deallocate();
         _uqBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();
      }
   } // end TVL1_FlowEstimator_Direct::deallocate()

   void
   TVL1_FlowEstimator_Direct::run(unsigned int I0_TexID, unsigned int I1_TexID)
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * ubuffer1 = _uqBuffer1Pyramid[level];
         RTT_Buffer * ubuffer2 = _uqBuffer2Pyramid[level];
         RTT_Buffer * pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * pbuffer2 = _pBuffer2Pyramid[level];

         float const lambda = _lambda;
         //float const lambda = _lambda * sqrtf(1 << level);

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
            //cout << "upsampleDisparities" << endl;
            upsampleDisparities(_uqBuffer2Pyramid[level+1]->textureID(), _pBuffer2Pyramid[level+1]->textureID(), 1.0f,
                                *ubuffer2, *pbuffer2);
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer = *_warpedBufferPyramid[level];

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         for (int iter = 0; iter < _nOuterIterations; ++iter)
         {
            warpImageWithFlowField(ubuffer2->textureID(), I0_TexID, I1_TexID, level, warpedBuffer);

            setupNormalizedProjection();

            _shader_uvq->parameter("timesteps", _cfg._tau_primal, _cfg._tau_dual);
            _shader_uvq->parameter("lambda_q", _cfg._lambdaScale);
            _shader_uvq->parameter("beta", _cfg._beta);
            _shader_p->parameter("timestep", _cfg._tau_dual);
            _shader_p->parameter("rcpLambda_p", (lambda / _cfg._lambdaScale));

            checkGLErrorsHere0();

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
               warpedBuffer.enableTexture(GL_TEXTURE2_ARB);

               _shader_uvq->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_uvq->disable();

               warpedBuffer.disableTexture(GL_TEXTURE2_ARB);

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(ubuffer1, ubuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_FlowEstimator_Direct::run()

//----------------------------------------------------------------------

   void
   TVL1_FlowEstimator_Bregman::allocate(int W, int H)
   {
      TVL1_FlowEstimatorBase::allocate(W, H);

      _shader_u_iter1 = new Cg_FragmentProgram("tvl1_flow_bregman_update_u_iter1");
      _shader_u_iter1->setProgramFromFile("OpticalFlow/tvl1_flow_bregman_update_u_iter1.cg");
      _shader_u_iter1->compile();

      _shader_u_iterN = new Cg_FragmentProgram("tvl1_flow_bregman_update_u_iterN");
      _shader_u_iterN->setProgramFromFile("OpticalFlow/tvl1_flow_bregman_update_u_iterN.cg");
      _shader_u_iterN->compile();

#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
      _shader_p = new Cg_FragmentProgram("tvl1_flow_bregman_update_p");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_bregman_update_p.cg");
      _shader_p->compile();

      _shader_b = new Cg_FragmentProgram("tvl1_flow_bregman_update_b");
      _shader_b->setProgramFromFile("OpticalFlow/tvl1_flow_bregman_update_b.cg");
      _shader_b->compile();
#else
      _shader_pb = new Cg_FragmentProgram("tvl1_flow_bregman_update_pb");
      _shader_pb->setProgramFromFile("OpticalFlow/tvl1_flow_bregman_update_pb.cg");
      _shader_pb->compile();
#endif

      _shader_vd = new Cg_FragmentProgram("tvl1_flow_bregman_update_vd");
      _shader_vd->setProgramFromFile("OpticalFlow/tvl1_flow_bregman_update_vd.cg");
      _shader_vd->compile();

      _uBuffer1Pyramid.resize(_nLevels);
      _uBuffer2Pyramid.resize(_nLevels);
      _pBufferPyramid.resize(_nLevels);
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
      _bBufferPyramid.resize(_nLevels);
#endif
      _vdBuffer1Pyramid.resize(_nLevels);
      _vdBuffer2Pyramid.resize(_nLevels);

      char const * uTexSpec  = _uvBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";
      char const * pTexSpec  = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";
      char const * vdTexSpec = _uvBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";

      for (int level = 0; level < _nLevels; ++level)
      {
         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         _uBuffer1Pyramid[level] = new RTT_Buffer(uTexSpec, "ubuffer1");
         _uBuffer1Pyramid[level]->allocate(w, h);
         _uBuffer2Pyramid[level] = new RTT_Buffer(uTexSpec, "ubuffer2");
         _uBuffer2Pyramid[level]->allocate(w, h);
         _pBufferPyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer");
         _pBufferPyramid[level]->allocate(w, h);
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
         _bBufferPyramid[level] = new RTT_Buffer(pTexSpec, "bbuffer");
         _bBufferPyramid[level]->allocate(w, h);
#endif

         _vdBuffer1Pyramid[level] = new RTT_Buffer(vdTexSpec, "vdBuffer1");
         _vdBuffer1Pyramid[level]->allocate(w, h);
         _vdBuffer2Pyramid[level] = new RTT_Buffer(vdTexSpec, "vdBuffer2");
         _vdBuffer2Pyramid[level]->allocate(w, h);
      } // end for (level)

#if defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
      _b1TexPyramid.resize(_nLevels);
      _b2TexPyramid.resize(_nLevels);
      _fboPB1Pyramid.resize(_nLevels);
      _fboPB2Pyramid.resize(_nLevels);

      for (int level = 0; level < _nLevels; ++level)
      {
         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         _b1TexPyramid[level] = new ImageTexture2D("b1Tex");
         _b1TexPyramid[level]->allocateID();
         _b1TexPyramid[level]->reserve(w, h, pTexSpec);

         _b2TexPyramid[level] = new ImageTexture2D("b1Tex");
         _b2TexPyramid[level]->allocateID();
         _b2TexPyramid[level]->reserve(w, h, pTexSpec);

         _fboPB1Pyramid[level] = new FrameBufferObject("fboPB1");
         _fboPB1Pyramid[level]->allocate();
         _fboPB2Pyramid[level] = new FrameBufferObject("fboPB2");
         _fboPB2Pyramid[level]->allocate();

         _fboPB1Pyramid[level]->makeCurrent();
         _fboPB1Pyramid[level]->attachTexture2D(_pBufferPyramid[level]->getTexture(),
                                                GL_COLOR_ATTACHMENT0_EXT, 0);
         _fboPB1Pyramid[level]->attachTexture2D(*_b1TexPyramid[level],
                                                GL_COLOR_ATTACHMENT1_EXT, 0);

         _fboPB2Pyramid[level]->makeCurrent();
         _fboPB2Pyramid[level]->attachTexture2D(_pBufferPyramid[level]->getTexture(),
                                                GL_COLOR_ATTACHMENT0_EXT, 0);
         _fboPB2Pyramid[level]->attachTexture2D(*_b2TexPyramid[level],
                                                GL_COLOR_ATTACHMENT1_EXT, 0);
      } // end for (level)
#endif
   } // end TVL1_FlowEstimator_Bregman::allocate()

   void
   TVL1_FlowEstimator_Bregman::deallocate()
   {
      TVL1_FlowEstimatorBase::deallocate();

#if defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
      for (int level = 0; level < _nLevels; ++level)
      {
         _fboPB1Pyramid[level]->deallocate();
         delete _fboPB1Pyramid[level];
         _fboPB2Pyramid[level]->deallocate();
         delete _fboPB2Pyramid[level];

         _b1TexPyramid[level]->deallocateID();
         delete _b1TexPyramid[level];
         _b2TexPyramid[level]->deallocateID();
         delete _b2TexPyramid[level];
      } // end for (level)
#endif

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBuffer1Pyramid[level]->deallocate();
         _uBuffer2Pyramid[level]->deallocate();
         _pBufferPyramid[level]->deallocate();
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
         _bBufferPyramid[level]->deallocate();
#endif
         _vdBuffer1Pyramid[level]->deallocate();
         _vdBuffer2Pyramid[level]->deallocate();
      }
   } // end TVL1_FlowEstimator_Bregman::deallocate()

   void
   TVL1_FlowEstimator_Bregman::run(unsigned int I0_TexID, unsigned int I1_TexID)
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * ubuffer1 = _uBuffer1Pyramid[level];
         RTT_Buffer * ubuffer2 = _uBuffer2Pyramid[level];
         RTT_Buffer * pbuffer  = _pBufferPyramid[level];
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
         RTT_Buffer * bbuffer  = _bBufferPyramid[level];
#else
         ImageTexture2D * b1Tex = _b1TexPyramid[level];
         ImageTexture2D * b2Tex = _b2TexPyramid[level];
         FrameBufferObject * fboB1 = _fboPB1Pyramid[level];
         FrameBufferObject * fboB2 = _fboPB2Pyramid[level];
#endif
         RTT_Buffer * vdbuffer1 = _vdBuffer1Pyramid[level];
         RTT_Buffer * vdbuffer2 = _vdBuffer2Pyramid[level];

         float const lambda = _lambda;
         //float const lambda = _lambda * sqrtf(1 << level);

         glClearColor(0, 0, 0, 0);
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
         if (level == _nLevels-1)
         {
            ubuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            pbuffer->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            bbuffer->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            vdbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
         }
         else
         {
            upsampleBuffer(_uBuffer2Pyramid[level+1]->textureID(), 2.0f, ubuffer2->getFBO());
            upsampleBuffer(_pBufferPyramid[level+1]->textureID(), 2.0f, pbuffer->getFBO());
            upsampleBuffer(_bBufferPyramid[level+1]->textureID(), 2.0f, bbuffer->getFBO());
            upsampleBuffer(_vdBuffer2Pyramid[level+1]->textureID(), 2.0f, vdbuffer2->getFBO());
         }
#else
         if (level == _nLevels-1)
         {
            ubuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            pbuffer->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            vdbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            fboB2->activate();
            glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
         }
         else
         {
            upsampleBuffer(_uBuffer2Pyramid[level+1]->textureID(), 2.0f, ubuffer2->getFBO());
            upsampleBuffer(_vdBuffer2Pyramid[level+1]->textureID(), 2.0f, vdbuffer2->getFBO());
            upsampleBuffers(_pBufferPyramid[level+1]->textureID(), _b2TexPyramid[level+1]->textureID(),
                            2.0, 2.0, *fboB2);
         }
#endif

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer = *_warpedBufferPyramid[level];

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         for (int iter = 0; iter < _nOuterIterations; ++iter)
         {
            warpImageWithFlowField(ubuffer2->textureID(), I0_TexID, I1_TexID, level, warpedBuffer);

            setupNormalizedProjection();

#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
            _shader_p->parameter("rcpMu", 1.0f / _mu);
#else
            _shader_pb->parameter("rcpMu", 1.0f / _mu);
#endif
            _shader_vd->parameter("lambda_over_mu", (lambda / _mu));

            checkGLErrorsHere0();

            for (int k = 0; k < _nInnerIterations /* * sqrtf(resizeFactor) */; ++k)
            {
               // Update u
               ubuffer1->activate();
               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               vdbuffer2->enableTexture(GL_TEXTURE1_ARB);
               pbuffer->enableTexture(GL_TEXTURE2_ARB);
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
               bbuffer->enableTexture(GL_TEXTURE3_ARB);
#else
               b2Tex->enable(GL_TEXTURE3_ARB);
#endif

               _shader_u_iter1->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_u_iter1->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               vdbuffer2->disableTexture(GL_TEXTURE1_ARB);
               pbuffer->disableTexture(GL_TEXTURE2_ARB);
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
               bbuffer->disableTexture(GL_TEXTURE3_ARB);
#else
               b2Tex->disable(GL_TEXTURE3_ARB);
#endif

               std::swap(ubuffer1, ubuffer2);

               // Subsequent Jacobi iterations for u
               // Div(b-p)+v-d is stored in additional color channels in ubuffer
               _shader_u_iterN->enable();
               for (int jacobiIter = 0; jacobiIter < _nJacobiIters; ++jacobiIter)
               {
                  ubuffer1->activate();
                  ubuffer2->enableTexture(GL_TEXTURE0_ARB);

                  renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);

                  std::swap(ubuffer1, ubuffer2);
               }
               _shader_u_iterN->disable();

               // Update p
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
               pbuffer->activate();
               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               bbuffer->enableTexture(GL_TEXTURE1_ARB);

               _shader_p->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_p->disable();

               bbuffer->disableTexture(GL_TEXTURE1_ARB);

               // Update b
               bbuffer->activate();
               pbuffer->enableTexture(GL_TEXTURE1_ARB);

               glBlendFunc(GL_ONE, GL_ONE);
               glBlendEquation(GL_FUNC_ADD);
               glEnable(GL_BLEND);
               _shader_b->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_b->disable();
               glDisable(GL_BLEND);

               pbuffer->disableTexture(GL_TEXTURE1_ARB);
#else
               fboB1->activate();
               GLenum const targetBuffers[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
               glDrawBuffersARB(2, targetBuffers);

               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               b2Tex->enable(GL_TEXTURE1_ARB);

               _shader_pb->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_pb->disable();

               glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

               b2Tex->disable(GL_TEXTURE1_ARB);

               std::swap(fboB1, fboB2);
               std::swap(b1Tex, b2Tex);
#endif

               // Update v and d
               vdbuffer1->activate();
               vdbuffer2->enableTexture(GL_TEXTURE1_ARB);
               warpedBuffer.enableTexture(GL_TEXTURE2_ARB);

               _shader_vd->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_vd->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               vdbuffer2->disableTexture(GL_TEXTURE1_ARB);
               warpedBuffer.disableTexture(GL_TEXTURE2_ARB);

               std::swap(vdbuffer1, vdbuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_FlowEstimator_Bregman::run()

//----------------------------------------------------------------------

   void
   TVL1_FlowEstimator_DR::allocate(int W, int H)
   {
      TVL1_FlowEstimatorBase::allocate(W, H);

      _shader_u = new Cg_FragmentProgram("tvl1_flow_DR_update_u");
      _shader_u->setProgramFromFile("OpticalFlow/tvl1_flow_DR_update_u.cg");
      _shader_u->compile();

      _shader_v = new Cg_FragmentProgram("tvl1_flow_DR_update_v");
      _shader_v->setProgramFromFile("OpticalFlow/tvl1_flow_DR_update_v.cg");
      _shader_v->compile();

      _shader_p = new Cg_FragmentProgram("tvl1_flow_DR_update_p");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_DR_update_p.cg");
      _shader_p->compile();

      _uBufferPyramid.resize(_nLevels);
      _vBuffer1Pyramid.resize(_nLevels);
      _vBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);

      char const * uTexSpec = _uvBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
      char const * vTexSpec = _uvBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
      char const * pTexSpec  = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";

      for (int level = 0; level < _nLevels; ++level)
      {
         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         _uBufferPyramid[level] = new RTT_Buffer(uTexSpec, "ubuffer");
         _uBufferPyramid[level]->allocate(w, h);
         _vBuffer1Pyramid[level] = new RTT_Buffer(vTexSpec, "vbuffer1");
         _vBuffer1Pyramid[level]->allocate(w, h);
         _vBuffer2Pyramid[level] = new RTT_Buffer(vTexSpec, "vbuffer2");
         _vBuffer2Pyramid[level]->allocate(w, h);

         _pBuffer1Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer1");
         _pBuffer1Pyramid[level]->allocate(w, h);
         _pBuffer2Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer2");
         _pBuffer2Pyramid[level]->allocate(w, h);
      } // end for (level)
   } // end TVL1_FlowEstimator_DR::allocate()

   void
   TVL1_FlowEstimator_DR::deallocate()
   {
      TVL1_FlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBufferPyramid[level]->deallocate();
         _vBuffer1Pyramid[level]->deallocate();
         _vBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();
      }
   } // end TVL1_FlowEstimator_DR::deallocate()

   void
   TVL1_FlowEstimator_DR::run(unsigned int I0_TexID, unsigned int I1_TexID)
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer *  ubuffer  = _uBufferPyramid[level];
         RTT_Buffer * &vbuffer1 = _vBuffer1Pyramid[level];
         RTT_Buffer * &vbuffer2 = _vBuffer2Pyramid[level];
         RTT_Buffer * &pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * &pbuffer2 = _pBuffer2Pyramid[level];

         if (level == _nLevels-1)
         {
            glClearColor(0, 0, 0, 0);
            ubuffer->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            vbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
            pbuffer2->activate();
            glClear(GL_COLOR_BUFFER_BIT);
         }
         else
         {
            upsampleDisparities(_uBufferPyramid[level+1]->textureID(), _pBuffer2Pyramid[level+1]->textureID(), 1.0f,
                                *ubuffer, *pbuffer2);
            upsampleBuffer(_vBuffer2Pyramid[level+1]->textureID(), 2.0f, vbuffer2->getFBO());
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer = *_warpedBufferPyramid[level];

         float const theta  = _cfg._theta;
         //float const theta  = _cfg._theta / float(1 << level);
         float const lambda = _lambda;
         //float const lambda = _lambda * sqrtf(1 << level);

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         for (int iter = 0; iter < _nOuterIterations; ++iter)
         {
            warpImageWithFlowField(ubuffer->textureID(), I0_TexID, I1_TexID, level, warpedBuffer);

            setupNormalizedProjection();

            _shader_u->parameter("theta", theta);
            _shader_v->parameter("lambda_theta", lambda*theta);
            _shader_p->parameter("timestep_over_theta", _cfg._tau/theta);

            checkGLErrorsHere0();

            for (int k = 0; k < _nInnerIterations /* * sqrtf(resizeFactor) */; ++k)
            {
               vbuffer1->activate();
               ubuffer->enableTexture(GL_TEXTURE0_ARB);
               vbuffer2->enableTexture(GL_TEXTURE1_ARB);
               warpedBuffer.enableTexture(GL_TEXTURE3_ARB);

               _shader_v->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_v->disable();

               ubuffer->disableTexture(GL_TEXTURE0_ARB);
               vbuffer2->disableTexture(GL_TEXTURE1_ARB);
               warpedBuffer.disableTexture(GL_TEXTURE3_ARB);

               std::swap(vbuffer1, vbuffer2);

               for (int l = 0; l < _cfg._nROF_Iterations; ++l)
               {
                  ubuffer->activate();
                  vbuffer2->enableTexture(GL_TEXTURE1_ARB);
                  pbuffer2->enableTexture(GL_TEXTURE2_ARB);

                  _shader_u->enable();
                  renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
                  _shader_u->disable();

                  vbuffer2->disableTexture(GL_TEXTURE1_ARB);

                  pbuffer1->activate();
                  ubuffer->enableTexture(GL_TEXTURE0_ARB);
                  _shader_p->enable();
                  renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
                  _shader_p->disable();

                  ubuffer->disableTexture(GL_TEXTURE0_ARB);
                  pbuffer2->disableTexture(GL_TEXTURE2_ARB);

                  std::swap(pbuffer1, pbuffer2);
               } // end for (l)
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_FlowEstimator_DR::run()

//----------------------------------------------------------------------

   void
   TVL1_FlowEstimator_CLG::allocate(int W, int H)
   {
      TVL1_FlowEstimatorBase::allocate(W, H);

      _shader_u = new Cg_FragmentProgram("tvl1_flow_CLG_update_u");
      _shader_p = new Cg_FragmentProgram("tvl1_flow_CLG_update_p");

#ifdef INCLUDE_SOURCE
      _shader_u->setProgram(GlShaderStrings::tvl1_flow_CLG_update_u.c_str());
      _shader_p->setProgram(GlShaderStrings::tvl1_flow_new_update_p.c_str());
#else
      _shader_u->setProgramFromFile("OpticalFlow/tvl1_flow_CLG_update_u.cg");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_new_update_p.cg");
#endif

      _shader_u->compile();
      _shader_p->compile();

      _uBuffer1Pyramid.resize(_nLevels);
      _uBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);
      _tensorPyramid.resize(_nLevels);
      _tensor1Texs.resize(_nLevels);
      _tensor2Texs.resize(_nLevels);

      char const * uTexSpec = _uvBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";
      char const * pTexSpec  = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";

      for (int level = 0; level < _nLevels; ++level)
      {
         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         _uBuffer1Pyramid[level] = new RTT_Buffer(uTexSpec, "ubuffer1");
         _uBuffer1Pyramid[level]->allocate(w, h);
         _uBuffer2Pyramid[level] = new RTT_Buffer(uTexSpec, "ubuffer2");
         _uBuffer2Pyramid[level]->allocate(w, h);

         _pBuffer1Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer1");
         _pBuffer1Pyramid[level]->allocate(w, h);
         _pBuffer2Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer2");
         _pBuffer2Pyramid[level]->allocate(w, h);

         vector<unsigned char> pixels(w*h, 0);

         _tensor1Texs[level].allocateID();
         _tensor1Texs[level].reserve(w, h, TextureSpecification("rgb=16f tex2D"));
         //_tensor1Texs[level].overwriteWith(&pixels[0], 1);
         _tensor2Texs[level].allocateID();
         _tensor2Texs[level].reserve(w, h, TextureSpecification("rgb=16f tex2D"));
         //_tensor2Texs[level].overwriteWith(&pixels[0], 1);

         //_tensorPyramid[level] = new FrameBufferObject("tensorPyramid");
         _tensorPyramid[level].allocate();
         _tensorPyramid[level].makeCurrent();
         _tensorPyramid[level].attachTexture2D(_tensor1Texs[level], GL_COLOR_ATTACHMENT0_EXT);
         _tensorPyramid[level].attachTexture2D(_tensor2Texs[level], GL_COLOR_ATTACHMENT1_EXT);
      } // end for (level)
   } // end TVL1_FlowEstimator_CLG::allocate()

   void
   TVL1_FlowEstimator_CLG::deallocate()
   {
      TVL1_FlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBuffer1Pyramid[level]->deallocate();
         _uBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();

         _tensor1Texs[level].deallocateID();
         _tensor2Texs[level].deallocateID();
         _tensorPyramid[level].deallocate();
      }
   } // end TVL1_FlowEstimator_CLG::deallocate()

   void
   TVL1_FlowEstimator_CLG::run(unsigned int I0_TexID, unsigned int I1_TexID)
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * &ubuffer1 = _uBuffer1Pyramid[level];
         RTT_Buffer * &ubuffer2 = _uBuffer2Pyramid[level];
         RTT_Buffer * &pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * &pbuffer2 = _pBuffer2Pyramid[level];

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
            upsampleDisparities(_uBuffer2Pyramid[level+1]->textureID(), _pBuffer2Pyramid[level+1]->textureID(), 1.0f,
                                *ubuffer2, *pbuffer2);
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer = *_warpedBufferPyramid[level];

         float const lambda = _lambda;
         //float const lambda = _lambda * sqrtf(1 << level);
         float const theta = _cfg._theta;

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         for (int iter = 0; iter < _nOuterIterations; ++iter)
         {
            warpImageWithFlowField(ubuffer2->textureID(), I0_TexID, I1_TexID, level, warpedBuffer);
            accumulateStructureTensor(warpedBuffer.textureID(), level, _tensorPyramid[level]);

            setupNormalizedProjection();

#if 0
            _shader_u->parameter("lambda", lambda);
            _shader_u->parameter("tau_primal", _cfg._tau_primal);
            _shader_u->parameter("eps_primal", _cfg._eps_data*_cfg._eps_data, 1.0f / _cfg._eps_data);

            _shader_p->parameter("timestep", _cfg._tau_dual);
            //_shader_p->parameter("rcpLambda_p", 1.0f / lambda);
            _shader_p->parameter("rcpLambda_p", 1.0f);
#else
            _shader_u->parameter("lambda_theta", lambda * theta);
            _shader_u->parameter("theta", theta);
            _shader_u->parameter("eps_primal", _cfg._eps_data*_cfg._eps_data, 1.0f / _cfg._eps_data);

            _shader_p->parameter("timestep", 0.249f / theta);
            _shader_p->parameter("rcpLambda_p", 1.0f);
#endif

            checkGLErrorsHere0();

            for (int k = 0; k < _nInnerIterations /* * sqrtf(resizeFactor) */; ++k)
            {
               ubuffer1->activate();
               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);
               _tensor1Texs[level].enable(GL_TEXTURE2_ARB);
               _tensor2Texs[level].enable(GL_TEXTURE3_ARB);

               _shader_u->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_u->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               _tensor1Texs[level].disable(GL_TEXTURE2_ARB);
               _tensor2Texs[level].disable(GL_TEXTURE3_ARB);

               std::swap(ubuffer1, ubuffer2);

               pbuffer1->activate();
               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               _shader_p->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_p->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(pbuffer1, pbuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_FlowEstimator_CLG::run()

//----------------------------------------------------------------------

   void
   TVL1_FlowEstimator_CLG_PD::allocate(int W, int H)
   {
      TVL1_FlowEstimatorBase::allocate(W, H);

      _shader_u = new Cg_FragmentProgram("tvl1_flow_CLG_PD_update_u");
      _shader_u->setProgramFromFile("OpticalFlow/tvl1_flow_CLG_PD_update_u.cg");
      _shader_u->compile();

      _shader_q = new Cg_FragmentProgram("tvl1_flow_CLG_PD_update_q");
      _shader_q->setProgramFromFile("OpticalFlow/tvl1_flow_CLG_PD_update_q.cg");
      _shader_q->compile();

      _shader_p = new Cg_FragmentProgram("tvl1_flow_CLG_PD_update_p");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_new_update_p.cg");
      _shader_p->compile();

      _uBuffer1Pyramid.resize(_nLevels);
      _uBuffer2Pyramid.resize(_nLevels);
      _pBuffer1Pyramid.resize(_nLevels);
      _pBuffer2Pyramid.resize(_nLevels);
      _qBuffer1Pyramid.resize(_nLevels);
      _qBuffer2Pyramid.resize(_nLevels);

      _tensorPyramid.resize(_nLevels);
      _tensor1Texs.resize(_nLevels);
      _tensor2Texs.resize(_nLevels);

      _cholPyramid.resize(_nLevels);
      _chol1Texs.resize(_nLevels);
      _chol2Texs.resize(_nLevels);

      char const * uTexSpec = _uvBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";
      char const * pTexSpec  = _pBufferHighPrecision ? "rgba=32f tex2D" : "rgba=16f tex2D";
      char const * qTexSpec  = _pBufferHighPrecision ? "rgb=32f tex2D" : "rgb=16f tex2D";

      for (int level = 0; level < _nLevels; ++level)
      {
         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         _uBuffer1Pyramid[level] = new RTT_Buffer(uTexSpec, "ubuffer1");
         _uBuffer1Pyramid[level]->allocate(w, h);
         _uBuffer2Pyramid[level] = new RTT_Buffer(uTexSpec, "ubuffer2");
         _uBuffer2Pyramid[level]->allocate(w, h);

         _pBuffer1Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer1");
         _pBuffer1Pyramid[level]->allocate(w, h);
         _pBuffer2Pyramid[level] = new RTT_Buffer(pTexSpec, "pbuffer2");
         _pBuffer2Pyramid[level]->allocate(w, h);

         _qBuffer1Pyramid[level] = new RTT_Buffer(pTexSpec, "qbuffer1");
         _qBuffer1Pyramid[level]->allocate(w, h);
         _qBuffer2Pyramid[level] = new RTT_Buffer(pTexSpec, "qbuffer2");
         _qBuffer2Pyramid[level]->allocate(w, h);

         _tensor1Texs[level].allocateID();
         _tensor1Texs[level].reserve(w, h, TextureSpecification("rgb=32f tex2D"));
         _tensor2Texs[level].allocateID();
         _tensor2Texs[level].reserve(w, h, TextureSpecification("rgb=32f tex2D"));

         _tensorPyramid[level].allocate();
         _tensorPyramid[level].makeCurrent();
         _tensorPyramid[level].attachTexture2D(_tensor1Texs[level], GL_COLOR_ATTACHMENT0_EXT);
         _tensorPyramid[level].attachTexture2D(_tensor2Texs[level], GL_COLOR_ATTACHMENT1_EXT);

         _chol1Texs[level].allocateID();
         _chol1Texs[level].reserve(w, h, TextureSpecification("rgb=32f tex2D"));
         _chol2Texs[level].allocateID();
         _chol2Texs[level].reserve(w, h, TextureSpecification("rgb=32f tex2D"));

         _cholPyramid[level].allocate();
         _cholPyramid[level].makeCurrent();
         _cholPyramid[level].attachTexture2D(_chol1Texs[level], GL_COLOR_ATTACHMENT0_EXT);
         _cholPyramid[level].attachTexture2D(_chol2Texs[level], GL_COLOR_ATTACHMENT1_EXT);
      } // end for (level)
   } // end TVL1_FlowEstimator_CLG::allocate()

   void
   TVL1_FlowEstimator_CLG_PD::deallocate()
   {
      TVL1_FlowEstimatorBase::deallocate();

      for (int level = 0; level < _nLevels; ++level)
      {
         _uBuffer1Pyramid[level]->deallocate();
         _uBuffer2Pyramid[level]->deallocate();
         _pBuffer1Pyramid[level]->deallocate();
         _pBuffer2Pyramid[level]->deallocate();
         _qBuffer1Pyramid[level]->deallocate();
         _qBuffer2Pyramid[level]->deallocate();

         _tensor1Texs[level].deallocateID();
         _tensor2Texs[level].deallocateID();
         _tensorPyramid[level].deallocate();

         _chol1Texs[level].deallocateID();
         _chol2Texs[level].deallocateID();
         _cholPyramid[level].deallocate();
      }
   } // end TVL1_FlowEstimator_CLG_PD::deallocate()

   void
   TVL1_FlowEstimator_CLG_PD::run(unsigned int I0_TexID, unsigned int I1_TexID)
   {
      for (int level = _nLevels-1; level >= _startLevel; --level)
      {
         RTT_Buffer * &ubuffer1 = _uBuffer1Pyramid[level];
         RTT_Buffer * &ubuffer2 = _uBuffer2Pyramid[level];
         RTT_Buffer * &pbuffer1 = _pBuffer1Pyramid[level];
         RTT_Buffer * &pbuffer2 = _pBuffer2Pyramid[level];
         RTT_Buffer * &qbuffer1 = _qBuffer1Pyramid[level];
         RTT_Buffer * &qbuffer2 = _qBuffer2Pyramid[level];

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
            upsampleDisparities(_uBuffer2Pyramid[level+1]->textureID(), _pBuffer2Pyramid[level+1]->textureID(), 1.0f,
                                *ubuffer2, *pbuffer2);
            upsampleBuffer(_qBuffer2Pyramid[level+1]->textureID(), 1.0f, qbuffer2->getFBO());
         }

         int const w = _width / (1 << level);
         int const h = _height / (1 << level);

         RTT_Buffer& warpedBuffer = *_warpedBufferPyramid[level];

         float const lambda = _lambda;
         //float const lambda = _lambda * sqrtf(1 << level);

         float const ds = 1.0f / w;
         float const dt = 1.0f / h;

         for (int iter = 0; iter < _nOuterIterations; ++iter)
         {
            warpImageWithFlowField(ubuffer2->textureID(), I0_TexID, I1_TexID, level, warpedBuffer);
            accumulateStructureTensor(warpedBuffer.textureID(), level, _tensorPyramid[level]);
            computeCholeskyFromTensor(_tensor1Texs[level].textureID(), _tensor2Texs[level].textureID(),
                                      _cholPyramid[level]);

            setupNormalizedProjection();

            float const multiplier = 4.0f;

            _shader_u->parameter("lambda", lambda / multiplier);
            //_shader_u->parameter("lambda", 1.0f);
            _shader_u->parameter("tau_primal", _cfg._tau_primal);
            //_shader_u->parameter("eps_primal", _cfg._eps_data*_cfg._eps_data, 1.0f / _cfg._eps_data);

            _shader_q->parameter("tau_dual", _cfg._tau_dual);
            _shader_q->parameter("lambda", lambda / multiplier, 1.0/multiplier);
            //_shader_q->parameter("lambda", 1.0, 1.0f);
            _shader_q->parameter("eps_data", _cfg._eps_data / (multiplier*multiplier));

            _shader_p->parameter("timestep", _cfg._tau_dual);
            //_shader_p->parameter("rcpLambda_p", lambda);
            _shader_p->parameter("rcpLambda_p", 1.0f);

            checkGLErrorsHere0();

            for (int k = 0; k < _nInnerIterations /* * sqrtf(resizeFactor) */; ++k)
            {
               ubuffer1->activate();
               ubuffer2->enableTexture(GL_TEXTURE0_ARB);
               pbuffer2->enableTexture(GL_TEXTURE1_ARB);
               qbuffer2->enableTexture(GL_TEXTURE2_ARB);
               _chol1Texs[level].enable(GL_TEXTURE3_ARB);
               _chol2Texs[level].enable(GL_TEXTURE4_ARB);

               _shader_u->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_u->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);

               std::swap(ubuffer1, ubuffer2);

               qbuffer1->activate();
               ubuffer2->enableTexture(GL_TEXTURE0_ARB);

               _shader_q->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_q->disable();

               qbuffer2->disableTexture(GL_TEXTURE2_ARB);
               _chol1Texs[level].disable(GL_TEXTURE3_ARB);
               _chol2Texs[level].disable(GL_TEXTURE4_ARB);

               std::swap(qbuffer1, qbuffer2);

               pbuffer1->activate();

               _shader_p->enable();
               renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
               _shader_p->disable();

               ubuffer2->disableTexture(GL_TEXTURE0_ARB);
               pbuffer2->disableTexture(GL_TEXTURE1_ARB);

               std::swap(pbuffer1, pbuffer2);
            } // end for (k)
         } // end for (iter)
      } // end for (level)
   } // end TVL1_FlowEstimator_CLG_PD::run()

//----------------------------------------------------------------------

   void
   warpImageWithFlowField(unsigned int uv_tex, unsigned int I0_tex,
                          unsigned int I1_tex, int level, RTT_Buffer& dest)
   {
      static Cg_FragmentProgram * shader = 0;
      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::warpImageWithFlowField::shader");
#ifdef INCLUDE_SOURCE
         shader->setProgram(GlShaderStrings::flow_warp_image.c_str());
#else
         shader->setProgramFromFile("flow_warp_image.cg");
#endif
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

   void
   warpImageWithFlowFieldAndGain(unsigned int uv_tex, unsigned int I0_tex,
                                 unsigned int I1_tex, int level, FrameBufferObject& fbo)
   {
      static Cg_FragmentProgram * shader = 0;
      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::warpImageWithFlowFieldAndGain::shader");
#ifdef INCLUDE_SOURCE
         shader->setProgram(GlShaderStrings::flow_warp_image_with_gain.c_str());
#else
         shader->setProgramFromFile("flow_warp_image_with_gain.cg");
#endif
         shader->compile();
         checkGLErrorsHere0();
      }

      int const w = fbo.width();
      int const h = fbo.height();

      fbo.activate();
      GLenum const targetBuffers[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
      glDrawBuffersARB(2, targetBuffers);

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

      glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

      checkGLErrorsHere0();
   } // end warpImageWithFlowFieldAndGain()

   void
   accumulateStructureTensor(unsigned int derivTex, int level, FrameBufferObject& fbo)
   {
      static Cg_FragmentProgram * shader = 0;
      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::accumulateStructureTensor::shader");
#ifdef INCLUDE_SOURCE
         shader->setProgram(GlShaderStrings::flow_accumulate_tensor.c_str());
#else
         shader->setProgramFromFile("flow_accumulate_tensor.cg");
#endif
         shader->compile();
         checkGLErrorsHere0();
      }

      int const w = fbo.width();
      int const h = fbo.height();

      float const ds = 1.0f/w;
      float const dt = 1.0f/h;

      fbo.activate();
      GLenum const targetBuffers[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
      glDrawBuffersARB(2, targetBuffers);

      setupNormalizedProjection();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, derivTex);
      glEnable(GL_TEXTURE_2D);

      shader->enable();
      renderNormalizedQuad(GPU_SAMPLE_REVERSE_NEIGHBORS, ds, dt);
      shader->disable();

      glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
   } // end accumulateStructureTensor()

   void
   computeCholeskyFromTensor(unsigned int T1_tex, unsigned int T2_tex, FrameBufferObject& fbo)
   {
      static Cg_FragmentProgram * shader = 0;
      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::computeCholeskyFromTensor::shader");
#ifdef INCLUDE_SOURCE
         shader->setProgram(GlShaderStrings::flow_cholesky_3x3.c_str());
#else
         shader->setProgramFromFile("flow_cholesky_3x3.cg");
#endif
         shader->compile();
         checkGLErrorsHere0();
      }

      fbo.activate();
      GLenum const targetBuffers[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
      glDrawBuffersARB(2, targetBuffers);

      setupNormalizedProjection();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, T1_tex);
      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, T2_tex);
      glEnable(GL_TEXTURE_2D);

      shader->enable();
      renderNormalizedQuad();
      shader->disable();

      glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE1);
      glDisable(GL_TEXTURE_2D);
   } // end computeCholeskyFromTensor()

//----------------------------------------------------------------------

   void
   displayMotionAsColorLight(unsigned textureID, float scale, bool useSqrtMap)
   {
      static Cg_FragmentProgram * shader = 0;
      static ImageTexture2D colorMapTex;

      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::displayMotionAsColorLight::shader");
         char const * source =
            "void main(uniform sampler2D uv_tex : TEXTURE0, \n"
            "                  sampler2D color_map_tex : TEXTURE1, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "          uniform float  scale, \n"
            "          uniform float  useSqrtMap, \n"
            "              out float4 color_out : COLOR0) \n"
            "{ \n"
            "   float2 uv = tex2D(uv_tex, st0).xy; \n"
            "   float r = length(uv); \n"
            "   r = useSqrtMap ? sqrt(r) : r; \n"
            "   r *= scale; \n"
            "   float angle = (clamp(atan2(-uv.y, -uv.x) / 3.1415927, -1, 1) + 1) / 2; \n"
            "   float3 col = tex2D(color_map_tex, float2(angle, 0)).xyz; \n"
            "   color_out.xyz = (r <= 1) ? (1.0f + r*col - r) : (0.75f * col); \n"
            "   color_out.w = 1;"
            "} \n";
         shader->setProgram(source);
         shader->compile();
         checkGLErrorsHere0();

         colorMapTex.allocateID();

         V3D::Image<unsigned char> const im  = V3D::makeColorWheelImage();

         colorMapTex.reserve(im.width(), im.height(), TextureSpecification("rgb=8"));
         colorMapTex.overwriteWith(&im(0, 0, 0), &im(0, 0, 1), &im(0, 0, 2));
      }

      setupNormalizedProjection(true);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glEnable(GL_TEXTURE_2D);
      colorMapTex.enable(GL_TEXTURE1);
      shader->parameter("scale", scale);
      shader->parameter("useSqrtMap", useSqrtMap ? 1.0f : 0.0f);
      shader->enable();
      renderNormalizedQuad();
      shader->disable();
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      colorMapTex.disable(GL_TEXTURE1);
      checkGLErrorsHere0();
   } // end displayMotionAsColorLight()


   void
   displayMotionAsColorLight2(unsigned textureID, bool useSqrtMap)
   {
      static Cg_FragmentProgram * shader = 0;
      static ImageTexture2D colorMapTex;

      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::displayMotionAsColorLight2::shader");
         char const * source =
            "void main(uniform sampler2D uv_tex : TEXTURE0, \n"
            "                  sampler2D color_map_tex : TEXTURE1, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "          uniform float  useSqrtMap, \n"
            "              out float4 color_out : COLOR0) \n"
            "{ \n"
            "   float2 uv = tex2D(uv_tex, st0).xy; \n"
                 "   float mx = max(abs(uv.x), abs(uv.y)); \n"
                 "   float b = clamp(mx/255.0f, 0,1); \n"
                 "   mx = (mx == 0) ? 1.0 : mx; // no division by 0 \n"
                 "   float r = clamp((uv.x/mx)/2+.5f, 0,1); \n"
                 "   float g = clamp((uv.y/mx)/2+.5f, 0,1); \n"
            "   color_out.x = r; color_out.y = g; color_out.z = b; \n"
            "   color_out.w = 1;"
            "} \n";
         shader->setProgram(source);
         shader->compile();
         checkGLErrorsHere0();

         colorMapTex.allocateID();

         V3D::Image<unsigned char> const im  = V3D::makeColorWheelImage();

         colorMapTex.reserve(im.width(), im.height(), TextureSpecification("rgb=8"));
         colorMapTex.overwriteWith(&im(0, 0, 0), &im(0, 0, 1), &im(0, 0, 2));
      }

      setupNormalizedProjection(true);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glEnable(GL_TEXTURE_2D);
      colorMapTex.enable(GL_TEXTURE1);
      shader->parameter("useSqrtMap", useSqrtMap ? 1.0f : 0.0f);
      shader->enable();
      renderNormalizedQuad();
      shader->disable();
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      colorMapTex.disable(GL_TEXTURE1);
      checkGLErrorsHere0();
   }


   void
   displayMotionAsColorLight3(unsigned textureID)
   {
      static Cg_FragmentProgram * shader = 0;
      static ImageTexture2D colorMapTex;

      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::displayMotionAsColorLight3::shader");
         char const * source =
            "void main(uniform sampler2D uv_tex : TEXTURE0, \n"
            "                  sampler2D color_map_tex : TEXTURE1, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "              out float4 color_out : COLOR0) \n"
            "{ \n"
            "   float2 uv = tex2D(uv_tex, st0).xy; \n"
                 "   float mx = max(abs(uv.x), abs(uv.y)); \n"
                 "   float b = clamp(mx/255.0f, 0,1); \n"
                 "   mx = (mx == 0) ? 1.0 : mx; // no division by 0 \n"
                 "   float r = clamp((uv.x/mx)/2+.5f, 0,1); \n"
                 "   float g = clamp((uv.y/mx)/2+.5f, 0,1); \n"
            "   color_out.x = r; color_out.y = g; color_out.z = b; \n"
            "   color_out.w = 1;"
            "} \n";
         shader->setProgram(source);
         shader->compile();
         checkGLErrorsHere0();

         colorMapTex.allocateID();

         V3D::Image<unsigned char> const im  = V3D::makeColorWheelImage();

         colorMapTex.reserve(im.width(), im.height(), TextureSpecification("rgb=8"));
         colorMapTex.overwriteWith(&im(0, 0, 0), &im(0, 0, 1), &im(0, 0, 2));
      }

      setupNormalizedProjection(true);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glEnable(GL_TEXTURE_2D);
      colorMapTex.enable(GL_TEXTURE1);
      shader->enable();
      renderNormalizedQuad();
      shader->disable();
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      colorMapTex.disable(GL_TEXTURE1);
      checkGLErrorsHere0();
   }

   void
   displayMotionAsColorDark(unsigned textureID, float scale, bool invert)
   {
      static Cg_FragmentProgram * shader = 0;
      static ImageTexture2D colorMapTex;

      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::displayMotionAsColorDark::shader");
         char const * source =
            "void main(uniform sampler2D uv_tex : TEXTURE0, \n"
            "                  sampler2D color_map_tex : TEXTURE1, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "          uniform float  scale, \n"
            "          uniform float  invert, \n"
            "              out float4 color_out : COLOR0) \n"
            "{ \n"
            "   float2 uv = tex2D(uv_tex, st0).xy; \n"
            "   float r = length(uv); \n"
            "   float angle = (clamp(atan2(-uv.y, -uv.x) / 3.1415927, -1, 1) + 1) / 2; \n"
            "   float3 col = tex2D(color_map_tex, float2(angle, 0)).xyz; \n"
            "   color_out.xyz = scale * r * col; \n"
            "   color_out.xyz = invert ? (float3(1.0) - color_out.xyz) : color_out.xyz; \n"
            "   color_out.w = 1;"
            "} \n";
         shader->setProgram(source);
         shader->compile();
         checkGLErrorsHere0();

         colorMapTex.allocateID();

         V3D::Image<unsigned char> const im  = V3D::makeColorWheelImage();

         colorMapTex.reserve(im.width(), im.height(), TextureSpecification("rgb=8"));
         colorMapTex.overwriteWith(&im(0, 0, 0), &im(0, 0, 1), &im(0, 0, 2));
      }

      setupNormalizedProjection(true);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glEnable(GL_TEXTURE_2D);
      colorMapTex.enable(GL_TEXTURE1);
      shader->parameter("scale", scale);
      shader->parameter("invert", invert ? 1.0f : 0.0f);
      shader->enable();
      renderNormalizedQuad();
      shader->disable();
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      colorMapTex.disable(GL_TEXTURE1);
      checkGLErrorsHere0();
   } // end displayMotionAsColorDark()

} // end namespace V3D_GPU

#endif
