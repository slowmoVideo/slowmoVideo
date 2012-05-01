// -*- C++ -*-

#include "config.h"

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

   struct TVL1_FlowEstimator_WithGain : public TVL1_FlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float tau = 0.249f, float theta = 0.1f, float gamma = 1.0f, float delta = 1.0f)
                  : _tau(tau), _theta(theta), _gamma(gamma), _delta(delta)
               { }

               float _tau, _theta, _gamma, _delta;
         };


         TVL1_FlowEstimator_WithGain(int nLevels)
            : TVL1_FlowEstimatorBase(nLevels)
         {
            _shader_uv = 0;
            _shader_p  = 0;
            _shader_q  = 0;
         }

         ~TVL1_FlowEstimator_WithGain() { }

         void configure(Config const& cfg) { _cfg = cfg; }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexID, unsigned int I1_TexID);

         unsigned int getFlowFieldTextureID()
         {
            return _uBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         Config _cfg;

         Cg_FragmentProgram * _shader_uv;
         Cg_FragmentProgram * _shader_p;
         Cg_FragmentProgram * _shader_q;

         std::vector<RTT_Buffer *> _uBuffer1Pyramid, _uBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;
         std::vector<RTT_Buffer *> _qBuffer1Pyramid, _qBuffer2Pyramid;

         std::vector<FrameBufferObject> _warpedPyramid;
         std::vector<ImageTexture2D>    _warped1Texs, _warped2Texs;
   }; // end struct TVL1_FlowEstimator_WithGain

//----------------------------------------------------------------------

   struct TVL1_FlowEstimator_Direct : public TVL1_FlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float tau_primal = 0.7f, float tau_dual = 0.7f,
                      float beta = 0.0f, float lambdaScale = 1.0f)
                  : _tau_primal(tau_primal), _tau_dual(tau_dual), _beta(beta), _lambdaScale(lambdaScale)
               { }

               float _tau_primal, _tau_dual, _beta, _lambdaScale;
         };

         TVL1_FlowEstimator_Direct(int nLevels)
            : TVL1_FlowEstimatorBase(nLevels)
         {
            _shader_uvq = 0;
            _shader_p   = 0;
         }

         ~TVL1_FlowEstimator_Direct() { }

         void configure(Config const& cfg) { _cfg = cfg; }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexID, unsigned int I1_TexID);

         unsigned int getFlowFieldTextureID()
         {
            return _uqBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         Config _cfg;

         Cg_FragmentProgram * _shader_uvq;
         Cg_FragmentProgram * _shader_p;

         std::vector<RTT_Buffer *> _uqBuffer1Pyramid, _uqBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;
   }; // end struct TVL1_FlowEstimator_Direct

//----------------------------------------------------------------------

#define V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT 1

   struct TVL1_FlowEstimator_Bregman : public TVL1_FlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float mu = 1.0f, int nJacobiIters = 3)
                  : _mu(mu), _nJacobiIters(nJacobiIters)
               { }

               float _mu;
               int   _nJacobiIters;
         };

         TVL1_FlowEstimator_Bregman(int nLevels)
            : TVL1_FlowEstimatorBase(nLevels), _mu(1.0f)
         {
            _shader_u_iter1 = 0;
            _shader_u_iterN = 0;
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
            _shader_p  = 0;
            _shader_b  = 0;
#else
            _shader_pb = 0;
#endif
            _shader_vd = 0;
         }

         ~TVL1_FlowEstimator_Bregman() { }

         void configure(Config const& cfg)
         {
            _mu = cfg._mu;
            _nJacobiIters = cfg._nJacobiIters;
         }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexID, unsigned int I1_TexID);

         unsigned int getFlowFieldTextureID()
         {
            return _uBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         float _mu;
         int   _nJacobiIters;

         Cg_FragmentProgram * _shader_u_iter1;
         Cg_FragmentProgram * _shader_u_iterN;
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
         Cg_FragmentProgram * _shader_p;
         Cg_FragmentProgram * _shader_b;
#else
         Cg_FragmentProgram * _shader_pb;
#endif
         Cg_FragmentProgram * _shader_vd;

         std::vector<RTT_Buffer *> _uBuffer1Pyramid, _uBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBufferPyramid;
#if !defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
         std::vector<RTT_Buffer *> _bBufferPyramid;
#endif
         std::vector<RTT_Buffer *> _vdBuffer1Pyramid, _vdBuffer2Pyramid;

#if defined(V3D_TVL1_BREGMAN_FLOW_ENABLE_MRT)
         std::vector<ImageTexture2D *> _b1TexPyramid, _b2TexPyramid;
         std::vector<FrameBufferObject *> _fboPB1Pyramid, _fboPB2Pyramid;
#endif
   }; // end struct TVL1_FlowEstimator_Bregman

//----------------------------------------------------------------------

   // Douglas-Rachford like implementation of TV-L1 flow
   struct TVL1_FlowEstimator_DR : public TVL1_FlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float tau = 0.249f, float theta = 0.25f, int nROF_Iterations = 4)
                  : _tau(tau), _theta(theta), _nROF_Iterations(nROF_Iterations)
               { }

               float _tau, _theta;
               int   _nROF_Iterations;
         };

         TVL1_FlowEstimator_DR(int nLevels)
            : TVL1_FlowEstimatorBase(nLevels)
         {
            _shader_u = 0;
            _shader_v = 0;
            _shader_p = 0;
         }

         ~TVL1_FlowEstimator_DR() { }

         void configure(Config const& cfg)
         {
            _cfg = cfg;
         }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexID, unsigned int I1_TexID);

         unsigned int getFlowFieldTextureID()
         {
            return _uBufferPyramid[_startLevel]->textureID();
         }

         unsigned int getAlternateFlowFieldTextureID()
         {
            return _vBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         Config _cfg;

         Cg_FragmentProgram * _shader_u;
         Cg_FragmentProgram * _shader_v;
         Cg_FragmentProgram * _shader_p;

         std::vector<RTT_Buffer *> _uBufferPyramid, _vBuffer1Pyramid, _vBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;
   }; // end struct TVL1_FlowEstimator_DR

//----------------------------------------------------------------------

   // Combined local-global optical flow
   struct TVL1_FlowEstimator_CLG : public TVL1_FlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float theta = 0.1f, float eps_data = 0.01f)
                  : _theta(theta), _eps_data(eps_data)
               { }

               float _theta, _eps_data;
         };

         TVL1_FlowEstimator_CLG(int nLevels)
            : TVL1_FlowEstimatorBase(nLevels)
         {
            _shader_u = 0;
            _shader_p = 0;
         }

         ~TVL1_FlowEstimator_CLG() { }

         void configure(Config const& cfg)
         {
            _cfg = cfg;
         }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexID, unsigned int I1_TexID);

         unsigned int getFlowFieldTextureID()
         {
            return _uBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         Config _cfg;

         Cg_FragmentProgram * _shader_u;
         GLSL_FragmentProgram * _shader_p;

         std::vector<RTT_Buffer *> _uBuffer1Pyramid, _uBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;

         std::vector<FrameBufferObject> _tensorPyramid;
         std::vector<ImageTexture2D>    _tensor1Texs, _tensor2Texs;
   }; // end struct TVL1_FlowEstimator_CLG

//----------------------------------------------------------------------

   // Combined local-global optical flow, primal-dual implementation
   struct TVL1_FlowEstimator_CLG_PD : public TVL1_FlowEstimatorBase
   {
      public:
         struct Config
         {
               Config(float tau_primal = 0.7f, float tau_dual = 0.7f,
                      float eps_data = 0.1f, float eps_reg = 0.1f)
                  : _tau_primal(tau_primal), _tau_dual(tau_dual),
                    _eps_data(eps_data), _eps_reg(eps_reg)
               { }

               float _tau_primal, _tau_dual, _eps_data, _eps_reg;
         };

         TVL1_FlowEstimator_CLG_PD(int nLevels)
            : TVL1_FlowEstimatorBase(nLevels)
         {
            _shader_u = 0;
            _shader_p = 0;
         }

         ~TVL1_FlowEstimator_CLG_PD() { }

         void configure(Config const& cfg)
         {
            _cfg = cfg;
         }

         void allocate(int w, int h);
         void deallocate();

         void run(unsigned int I0_TexID, unsigned int I1_TexID);

         unsigned int getFlowFieldTextureID()
         {
            return _uBuffer2Pyramid[_startLevel]->textureID();
         }

      protected:
         Config _cfg;

         Cg_FragmentProgram * _shader_u;
         Cg_FragmentProgram * _shader_q;
         Cg_FragmentProgram * _shader_p;

         std::vector<RTT_Buffer *> _uBuffer1Pyramid, _uBuffer2Pyramid;
         std::vector<RTT_Buffer *> _qBuffer1Pyramid, _qBuffer2Pyramid;
         std::vector<RTT_Buffer *> _pBuffer1Pyramid, _pBuffer2Pyramid;

         std::vector<FrameBufferObject> _tensorPyramid, _cholPyramid;
         std::vector<ImageTexture2D>    _tensor1Texs, _tensor2Texs;
         std::vector<ImageTexture2D>    _chol1Texs, _chol2Texs;
   }; // end struct TVL1_FlowEstimator_CLG_PD

//----------------------------------------------------------------------

   void warpImageWithFlowField(unsigned int uv_tex, unsigned int I0_tex,
                               unsigned int I1_tex, int level, RTT_Buffer& dest);

   void warpImageWithFlowFieldAndGain(unsigned int uv_tex, unsigned int I0_tex,
                                      unsigned int I1_tex, int level, FrameBufferObject& fbo);

   void accumulateStructureTensor(unsigned int derivTex, int level, FrameBufferObject& fbo);
   void computeCholeskyFromTensor(unsigned int T1_tex, unsigned int T2_tex, FrameBufferObject& fbo);

   void displayMotionAsColorLight(unsigned textureID, float scale, bool useSqrtMap = false);
   void displayMotionAsColorLight2(unsigned textureID, bool useSqrtMap = false);
   void displayMotionAsColorLight3(unsigned textureID, bool useSqrtMap = false);
   void displayMotionAsColorDark(unsigned textureID, float scale, bool invert = false);

} // end namespace V3D_GPU

# endif

#endif // defined(V3D_GPU_FLOW_H)
