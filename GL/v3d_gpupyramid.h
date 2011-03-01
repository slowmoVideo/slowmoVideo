// -*- C++ -*-
#ifndef V3D_GPU_PYRAMID_H
#define V3D_GPU_PYRAMID_H

# if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "v3d_gpubase.h"

namespace V3D_GPU
{

   struct PyramidCreator
   {
         PyramidCreator()
            : _width(0), _height(0), _nLevels(0)
         { }

         ~PyramidCreator() { }

         int numberOfLevels() const { return _nLevels; }

         void allocate(int w, int h, int nLevels);
         void deallocate();

         void buildPyramidForGrayscaleImage(uchar const * image);

         void activateTarget(int level);

         unsigned int textureID() const { return _pyrTexID; }
         unsigned int sourceTextureID() const { return _srcTexID; }

      protected:
         int _width, _height, _nLevels;
         unsigned int _pyrFbIDs[8], _tmpFbIDs[8], _tmp2FbID;
         unsigned int _srcTexID, _pyrTexID, _tmpTexID, _tmpTex2ID;
   }; // end struct PyramidCreator

   struct PyramidWithDerivativesCreator
   {
         PyramidWithDerivativesCreator(bool useFP32 = false, char const * srcTexSpec = "r=8 noRTT")
            : _useFP32(useFP32), _width(0), _height(0), _nLevels(0), _srcTexSpec(srcTexSpec),
              _pass1HorizShader(0), _pass1VertShader(0), _pass2Shader(0)
         { }

         ~PyramidWithDerivativesCreator() { }

         int numberOfLevels() const { return _nLevels; }

         void allocate(int w, int h, int nLevels, int preSmoothingFilter = 0);
         void deallocate();

         void buildPyramidForGrayscaleImage(uchar const * image);
         void buildPyramidForGrayscaleImage(float const * image);

         void buildPyramidForGrayscaleTexture(unsigned int srcTexID);

         void activateTarget(int level);

         unsigned int textureID() const { return _pyrTexID; }
         unsigned int sourceTextureID() const { return _srcTex.textureID(); }

      protected:
         bool const _useFP32;
         int        _width, _height, _nLevels;
         unsigned int _pyrFbIDs[8], _tmpFbIDs[8], _tmp2FbID;
         unsigned int _pyrTexID, _tmpTexID, _tmpTex2ID;
         ImageTexture2D _srcTex;
         char const *   _srcTexSpec;

         Cg_FragmentProgram *_pass1HorizShader, *_pass1VertShader, *_pass2Shader;
   }; // end struct PyramidWithDerivativesCreator

} // end namespace V3D_GPU

# endif // defined(V3DLIB_GPGPU_ENABLE_CG)

#endif
