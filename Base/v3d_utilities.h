// -*- C++ -*-
// General utility procedures that do not fit anywhere else.
#ifndef V3D_UTILITIES_H
#define V3D_UTILITIES_H

#include "Base/v3d_image.h"
#include "Math/v3d_linear.h"

#include <cmath>

#ifdef _WIN32
#   define M_PI 3.14159265358979323846
#endif

namespace V3D
{

   inline Image<unsigned char>
   makeColorWheelImage()
   {
      // relative lengths of color transitions:
      // these are chosen based on perceptual similarity
      // (e.g. one can distinguish more shades between red and yellow 
      //  than between yellow and green)
#if 0
      int const RY = 15;
      int const YG = 6;
      int const GC = 4;
      int const CB = 11;
      int const BM = 13;
      int const MR = 6;
#else
      // Make a ramp of 64 colors (instead of 55).
      int const RY = 17;
      int const YG = 7;
      int const GC = 5;
      int const CB = 13;
      int const BM = 15;
      int const MR = 7;
#endif

      int const w = RY + YG + GC + CB + BM + MR;
      Image<unsigned char> I(w, 1, 3);

      int x = 0;
      for (int i = 0; i < RY; ++i, ++x) { I(x, 0, 0) = 255;          I(x, 0, 1) = 255*i/RY; I(x, 0, 2) = 0; }
      for (int i = 0; i < YG; ++i, ++x) { I(x, 0, 0) = 255-255*i/YG; I(x, 0, 1) = 255;      I(x, 0, 2) = 0; }
      for (int i = 0; i < GC; ++i, ++x) { I(x, 0, 0) = 0;            I(x, 0, 1) = 255;      I(x, 0, 2) = 255*i/GC; }
      for (int i = 0; i < CB; ++i, ++x) { I(x, 0, 0) = 0;            I(x, 0, 1) = 255-255*i/CB; I(x, 0, 2) = 255; }
      for (int i = 0; i < BM; ++i, ++x) { I(x, 0, 0) = 255*i/BM;     I(x, 0, 1) = 0;        I(x, 0, 2) = 255; }
      for (int i = 0; i < MR; ++i, ++x) { I(x, 0, 0) = 255;          I(x, 0, 1) = 0;        I(x, 0, 2) = 255-255*i/MR; }

      return I;
   } // end makeColorWheelImage()

   inline Vector3b
   getVisualColorForFlowVector(float u, float v, bool useSqrtMap = false)
   {
      using namespace std;

      static Image<unsigned char> const wheel = makeColorWheelImage();

      int const w = wheel.width();

      float r = sqrtf(u*u + v*v);
      if (useSqrtMap) r = sqrtf(r);
      float const phi = atan2f(-v, -u) / M_PI;
      float const fk = (phi + 1.0) / 2.0 * w;
      int   const k0 = (int)fk;
      int   const k1 = (k0 + 1) % w;
      float const f = fk - k0;

      Vector3b res;

      for (int b = 0; b < 3; ++b)
      {
         float const col0 = float(wheel(k0, 0, b));
         float const col1 = float(wheel(k1, 0, b));
         float col = (1-f)*col0 + f*col1;
         if (r <= 1)
	    col = 255.0f - r * (255.0f - col); // increase saturation with radius
         else
	    col *= .75f; // out of range
         res[b] = (int)col;
      } // end for (b)
      return res;
   } // end getVisualColorForFlowVector()

   inline Image<unsigned char>
   getVisualImageForFlowField(Image<float> const& u, Image<float> const& v, float scale,
                              bool useSqrtMap = false)
   {
      int const w = u.width();
      int const h = u.height();

      Image<unsigned char> res(w, h, 3);
      for (int y = 0; y < h; ++y)
         for (int x = 0; x < w; ++x)
         {
            Vector3b const c = getVisualColorForFlowVector(scale * u(x, y), scale * v(x, y), useSqrtMap);
            res(x, y, 0) = c[0];
            res(x, y, 1) = c[1];
            res(x, y, 2) = c[2];
         }
      return res;
   } // end getVisualImageForFlowField()

   template <typename T>
   inline void
   flipImageUpsideDown(Image<T>& I)
   {
      int const w = I.width();
      int const h = I.height();
      int const nChannels = I.numChannels();

      for (int c = 0; c < nChannels; ++c)
         for (int y = 0; y < h/2; ++y)
         {
            int const y1 = h - 1 - y;
            for (int x = 0; x < w; ++x)
               std::swap(I(x, y, c), I(x, y1, c));
         }
   } // end flipImageUpsideDown()

} // end namespace V3D

#endif
