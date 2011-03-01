// -*- C++ -*-

#ifndef V3D_GPU_FLOW_H
#define V3D_GPU_FLOW_H

# if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "GL/v3d_gpubase.h"
#include "GL/v3d_gpupyramid.h"

namespace V3D_GPU
{

   struct TVL1_FlowEstimatorBase
   {
         TVL1_FlowEstimatorBase(int nLevels)
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

         RTT_Buffer * getWarpedBuffer(int level) { return _warpedBufferPyramid[level]; }

      protected:
         bool _warpedBufferHighPrecision, _uvBufferHighPrecision, _pBufferHighPrecision;

         std::vector<RTT_Buffer *> _warpedBufferPyramid;

         int _nOuterIterations, _nInnerIterations;

         float _lambda;
         int _startLevel, _nLevels;
         int _width, _height;
   }; // end struct TVL1_FlowEstimatorBase

   struct TVL1_FlowEstimator_Relaxed : public TVL1_FlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float tau = 0.249f, float theta = 0.1f)
                  : _tau(tau), _theta(theta)
               { }

               float _tau, _theta;
         };


         TVL1_FlowEstimator_Relaxed(int nLevels)
            : TVL1_FlowEstimatorBase(nLevels), _theta(0.1f)
         {
            _tau = 0.249f;
            _shader_uv = 0;
            _shader_p  = 0;
         }

         ~TVL1_FlowEstimator_Relaxed() { }

         void configure(Config const& cfg)
         {
            _tau = cfg._tau;
            _theta = cfg._theta;
         }

         void setTheta(float theta) { _theta = theta; }
         void setBeta(float)        { /* No op */ }
         void setLambdaScale(float) { /* No op */ }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexID, unsigned int I1_TexID);

         void computeAlternateFlow();

         unsigned int getFlowFieldTextureID()
         {
            return _uBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         float _tau, _theta;

         Cg_FragmentProgram * _shader_uv;
         Cg_FragmentProgram * _shader_p;

         std::vector<RTT_Buffer *> _uBuffer1Pyramid, _uBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;
   }; // end struct TVL1_FlowEstimator_Relaxed

//----------------------------------------------------------------------

   void warpImageWithFlowField(unsigned int uv_tex, unsigned int I0_tex,
                               unsigned int I1_tex, int level, RTT_Buffer& dest);

   void displayMotionAsColorLight(unsigned textureID, float scale, bool useSqrtMap = false);
   void displayMotionAsColorDark(unsigned textureID, float scale, bool invert = false);

} // end namespace V3D_GPU

# endif

#endif // defined(V3D_GPU_FLOW_H)
