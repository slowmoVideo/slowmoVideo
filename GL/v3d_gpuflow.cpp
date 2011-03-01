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
      _shader_uv->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_update_uv.cg");
      _shader_uv->compile();

      _shader_p = new Cg_FragmentProgram("tvl1_flow_relaxed_update_p");
      _shader_p->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_update_p.cg");
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
         shader->setProgramFromFile("OpticalFlow/tvl1_flow_relaxed_compute_UV.cg");
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
   warpImageWithFlowField(unsigned int uv_tex, unsigned int I0_tex,
                          unsigned int I1_tex, int level, RTT_Buffer& dest)
   {
      static Cg_FragmentProgram * shader = 0;
      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("v3d_gpuflow::warpImageWithFlowField::shader");
         shader->setProgramFromFile("flow_warp_image.cg");
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
