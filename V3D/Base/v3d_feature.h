// -*- C++ -*-

#ifndef V3D_FEATURE_POINT_H
#define V3D_FEATURE_POINT_H

#include "Base/v3d_serialization.h"
#include "Math/v3d_linear.h"

namespace V3D
{

   struct SIFT_Feature
   {
         int      id;
         Vector2f position, direction;
         float    scale, magnitude;
         float    descriptor[128];

         void normalizeDescriptor(bool doCentering = false)
         {
            int const N = 128;
            float mean = 0.0f;

            if (doCentering)
            {
               for (int i = 0; i < N; ++i) mean += descriptor[i];
               mean /= N;
            }

            float var = 0.0f;
            for (int i = 0; i < N; ++i)
            {
               descriptor[i] -= mean;
               var += descriptor[i]*descriptor[i];
            }

            float const denom = 1.0f / sqrtf(var);
            for (int i = 0; i < N; ++i) descriptor[i] *= denom;
         }

         template <typename Archive> void serialize(Archive& ar)
         {
            SerializationScope<Archive> scope(ar, "SIFT_Feature");

            ar & id;
            ar & position[0] & position[1] & direction[0] & direction[1];
            ar & scale & magnitude;
            for (int i = 0; i < 128; ++i) ar & descriptor[i];
         }

         V3D_DEFINE_LOAD_SAVE(SIFT_Feature);
   }; // end struct SIFT_Feature

   V3D_DEFINE_IOSTREAM_OPS(SIFT_Feature);

} // end namespace V3D

#endif
