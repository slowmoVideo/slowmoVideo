// -*- C++ -*-

#include "config.h"

#ifndef V3D_GPU_FLOW_H
#define V3D_GPU_FLOW_H

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

//----------------------------------------------------------------------

   void warpImageWithFlowField(unsigned int uv_tex, unsigned int I0_tex,
                               unsigned int I1_tex, int level, RTT_Buffer& dest);

} // end namespace V3D_GPU

#endif // defined(V3D_GPU_FLOW_H)
