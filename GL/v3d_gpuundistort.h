// -*- C++ -*-
#ifndef V3D_GPU_UNDISTORT_H
#define V3D_GPU_UNDISTORT_H

# if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "GL/v3d_gpubase.h"

namespace V3D_GPU
{

   struct ParametricUndistortionFilter
   {
         ParametricUndistortionFilter()
            : _width(0), _height(0), _destBuffer("rgb=8", "ParametricUndistortionFilter::_destBuffer")
         { }

         void setDistortionParameters(double const fx, double const fy,
                                      double const radial[4], double const center[2])
         {
            _fx = fx;
            _fy = fy;
            _k1 = radial[0];
            _k2 = radial[1];
            _p1 = radial[2];
            _p2 = radial[3];
            _center[0] = center[0];
            _center[1] = center[1];
         }

         void setDistortionParameters(double const fx, double const fy, double const k1, double const k2,
                                      double const p1, double const p2, double const u, double const v)
         {
            _fx = fx;
            _fy = fy;
            _k1 = k1;
            _k2 = k2;
            _p1 = p1;
            _p2 = p2;
            _center[0] = u;
            _center[1] = v;
         }

         void allocate(int w, int h);
         void deallocate();

         void undistortColorImage(unsigned char const * image, unsigned char * result);
         void undistortIntensityImage(unsigned char const * image, unsigned char * result);

      protected:
         int _width, _height;
         double _fx, _fy;
         double _k1, _k2, _p1, _p2;
         double _center[2];

         ImageTexture2D _srcTex;
         RTT_Buffer  _destBuffer;
   }; // end struct ParametricUndistortionFilter

   struct Lookup1D_UndistortionFilter
   {
         Lookup1D_UndistortionFilter()
            : _width(0), _height(0), _destBuffer("rgb=8", "Lookup1D_UndistortionFilter::_destBuffer")
         { 
            _T_rect[0] = 1; _T_rect[1] = 0; _T_rect[2] = 0;
            _T_rect[3] = 0; _T_rect[4] = 1; _T_rect[5] = 0;
            _T_rect[6] = 0; _T_rect[7] = 0; _T_rect[8] = 1;
         }

         void allocate(int w, int h);
         void deallocate();

         void setDistortionCenter(float cx, float cy)
         {
            _center[0] = cx;
            _center[1] = cy;
         }

         void setLookupTable(float pixelSize, int const lutSize, float const radialOffsets[]);

         void undistortColorImage(unsigned char const * image, unsigned char * result);
         void undistortIntensityImage(unsigned char const * image, unsigned char * result);
         void setRectifyingHomography(float const T_rect[9]);

      protected:
         int   _width, _height, _lutSize;
         float _center[2];
         float _pixelSize;
         float _T_rect[9];

         ImageTexture2D _srcTex;
         RTT_Buffer     _destBuffer;
         unsigned int   _lut1DTexID;
   }; // end struct Lookup1D_UndistortionFilter


} // end namespace V3D_GPU

# endif // defined(V3DLIB_GPGPU_ENABLE_CG)

#endif
