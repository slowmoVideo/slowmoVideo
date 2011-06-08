// -*- C++ -*-

#ifndef V3D_GPU_COLOR_FLOW_H
#define V3D_GPU_COLOR_FLOW_H

# if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "GL/v3d_gpubase.h"
#include "GL/v3d_gpuflow.h"

namespace V3D_GPU
{

   struct TVL1_ColorFlowEstimatorBase
   {
         TVL1_ColorFlowEstimatorBase(int nLevels)
            : _warpedBufferHighPrecision(true),
              _uvBufferHighPrecision(true),
              _pBufferHighPrecision(false), // fp16 is usually enough for p
              _nOuterIterations(1), _nInnerIterations(50), _startLevel(0), _nLevels(nLevels),
              _width(-1), _height(-1)
         { }

         void setLambda(float lambda)        { _lambda = lambda; }
         void setOuterIterations(int nIters) { _nOuterIterations = nIters; }
         void setInnerIterations(int nIters) { _nInnerIterations = nIters; }
         void setStartLevel(int startLevel)  { _startLevel = startLevel; }

         // Must be called before allocate() to have an effect.
         void configurePrecision(bool warpedBufferHighPrecision,
                                 bool uvBufferHighPrecision,
                                 bool pBufferHighPrecision)
         {
            _warpedBufferHighPrecision = warpedBufferHighPrecision;
            _uvBufferHighPrecision     = uvBufferHighPrecision;
            _pBufferHighPrecision      = pBufferHighPrecision;
         }

         void allocate(int w, int h);
         void deallocate();

         RTT_Buffer * getWarpedBuffer(int channel, int level)
         {
            return _warpedBufferPyramids[channel][level];
         }

      protected:
         bool _warpedBufferHighPrecision, _uvBufferHighPrecision, _pBufferHighPrecision;

         std::vector<RTT_Buffer *> _warpedBufferPyramids[3];

         int _nOuterIterations, _nInnerIterations;

         float _lambda;
         int _startLevel, _nLevels;
         int _width, _height;
   }; // end struct TVL1_ColorFlowEstimatorBase

//----------------------------------------------------------------------

   struct TVL1_ColorFlowEstimator_Direct : public TVL1_ColorFlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float tau_primal = 0.7f, float tau_dual = 0.7f, float lambdaScale = 1.0f)
                  : _tau_primal(tau_primal), _tau_dual(tau_dual), _lambdaScale(lambdaScale)
               { }

               float _tau_primal, _tau_dual, _lambdaScale;
         };

         TVL1_ColorFlowEstimator_Direct(int nLevels)
            : TVL1_ColorFlowEstimatorBase(nLevels)
         {
            _shader_uv = 0;
            _shader_q  = 0;
            _shader_p  = 0;
         }

         ~TVL1_ColorFlowEstimator_Direct() { }

         void configure(Config const& cfg) { _cfg = cfg; }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexIDs[3], unsigned int I1_TexIDs[3]);

         unsigned int getFlowFieldTextureID()
         {
            return _uBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         Config _cfg;

         Cg_FragmentProgram *_shader_uv, *_shader_q, *_shader_p;

         std::vector<RTT_Buffer *> _uBuffer1Pyramid, _uBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;
         std::vector<RTT_Buffer *> _qBuffer1Pyramid, _qBuffer2Pyramid;
   }; // end struct TVL1_ColorFlowEstimator_Direct

//----------------------------------------------------------------------

   struct TVL1_ColorFlowEstimator_New : public TVL1_ColorFlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float tau_primal = 0.7f, float tau_dual = 0.7f, float lambdaScale = 1.0f)
                  : _tau_primal(tau_primal), _tau_dual(tau_dual), _lambdaScale(lambdaScale)
               { }

               float _tau_primal, _tau_dual, _lambdaScale;
         };

         TVL1_ColorFlowEstimator_New(int nLevels)
            : TVL1_ColorFlowEstimatorBase(nLevels)
         {
            _shader_uv = 0;
            _shader_q  = 0;
            _shader_p  = 0;
         }

         ~TVL1_ColorFlowEstimator_New() { }

         void configure(Config const& cfg) { _cfg = cfg; }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexIDs[3], unsigned int I1_TexIDs[3]);

         unsigned int getFlowFieldTextureID()
         {
            return _uBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         Config _cfg;

         Cg_FragmentProgram *_shader_uv, *_shader_q, *_shader_p;

         std::vector<RTT_Buffer *> _uBuffer1Pyramid, _uBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;
         std::vector<RTT_Buffer *> _qBuffer1Pyramid, _qBuffer2Pyramid;
   }; // end struct TVL1_ColorFlowEstimator_New

//----------------------------------------------------------------------

   // Quadratic relaxation approach
   struct TVL1_ColorFlowEstimator_QR : public TVL1_ColorFlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float tau = 0.249f, float theta = 0.1f)
                  : _tau(tau), _theta(theta)
               { }

               float _tau, _theta;
         };

         TVL1_ColorFlowEstimator_QR(int nLevels)
            : TVL1_ColorFlowEstimatorBase(nLevels)
         {
            _shader_uv = 0;
            _shader_p  = 0;
         }

         ~TVL1_ColorFlowEstimator_QR() { }

         void configure(Config const& cfg) { _cfg = cfg; }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexIDs[3], unsigned int I1_TexIDs[3]);

         unsigned int getFlowFieldTextureID()
         {
            return _uBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         Config _cfg;

         Cg_FragmentProgram *_shader_uv, *_shader_p;

         std::vector<RTT_Buffer *> _uBuffer1Pyramid, _uBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;
   }; // end struct TVL1_ColorFlowEstimator_QR

} // end namespace V3D_GPU

# endif

#endif // defined(V3D_GPU_FLOW_H)
