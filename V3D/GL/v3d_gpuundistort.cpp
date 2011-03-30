#include "v3d_gpuundistort.h"

#include <GL/glew.h>

#if defined(V3DLIB_GPGPU_ENABLE_CG)

using namespace V3D_GPU;

namespace
{

   Cg_FragmentProgram * parametricShader = 0;
   Cg_FragmentProgram * radialLUTShader = 0;

}

namespace V3D_GPU
{

   void
   ParametricUndistortionFilter::allocate(int w, int h)
   {
      _width = w;
      _height = h;

      _srcTex.allocateID();
      _srcTex.reserve(w, h, TextureSpecification("rgb=8"));
      _srcTex.bind(GL_TEXTURE0);
      glTexParameteri(_srcTex.textureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(_srcTex.textureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      _destBuffer.allocate(w, h);

      if (parametricShader == 0)
      {
         parametricShader = new Cg_FragmentProgram("ParametricUndistortionFilter::parametricShader");
         parametricShader->setProgramFromFile("undistort_parametric.cg");
#if 0
         char const * cgArgs[] = { "-bestprecision", "-nofastmath", "-nofastprecision", 0 };
         parametricShader->compile(cgArgs);
#else
         parametricShader->compile();
#endif
         checkGLErrorsHere0();
      }
   }

   void
   ParametricUndistortionFilter::deallocate()
   {
      _srcTex.deallocateID();
      _destBuffer.deallocate();
   }

   void
   ParametricUndistortionFilter::undistortColorImage(unsigned char const * image, unsigned char * result)
   {
      _srcTex.overwriteWith(image, 3);

      _destBuffer.activate();
      setupNormalizedProjection();

      parametricShader->parameter("f", _fx, _fy, 1.0f/_fx, 1.0f/_fy);
      parametricShader->parameter("k", _k1, _k2);
      parametricShader->parameter("p", _p1, _p2);
      parametricShader->parameter("center", _center[0], _center[1]);
      parametricShader->parameter("wh", _width, _height, 1.0f/_width, 1.0f/_height);

      _srcTex.enable(GL_TEXTURE0);

      parametricShader->enable();
#if 0
      renderNormalizedQuad();
#else
      glBegin(GL_TRIANGLES);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 0);
      glVertex2f(0, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 2*_width, 0);
      glVertex2f(2, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 2*_height);
      glVertex2f(0, 2);
      glEnd();
#endif
      parametricShader->disable();

      _srcTex.disable(GL_TEXTURE0);

      glReadPixels(0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, result);
   }

   void
   ParametricUndistortionFilter::undistortIntensityImage(unsigned char const * image, unsigned char * result)
   {
      _srcTex.overwriteWith(image, 1);

      _destBuffer.activate();
      setupNormalizedProjection();

      parametricShader->parameter("f", _fx, _fy, 1.0f/_fx, 1.0f/_fy);
      parametricShader->parameter("k", _k1, _k2);
      parametricShader->parameter("p", _p1, _p2);
      parametricShader->parameter("center", _center[0], _center[1]);
      parametricShader->parameter("wh", _width, _height, 1.0f/_width, 1.0f/_height);

      _srcTex.enable(GL_TEXTURE0);

      parametricShader->enable();
#if 0
      renderNormalizedQuad();
#else
      glBegin(GL_TRIANGLES);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 0);
      glVertex2f(0, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 2*_width, 0);
      glVertex2f(2, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 2*_height);
      glVertex2f(0, 2);
      glEnd();
#endif
      parametricShader->disable();

      _srcTex.disable(GL_TEXTURE0);

      glReadPixels(0, 0, _width, _height, GL_RED, GL_UNSIGNED_BYTE, result);
   }

//----------------------------------------------------------------------

   void
   Lookup1D_UndistortionFilter::allocate(int w, int h)
   {
      _width = w;
      _height = h;

      _srcTex.allocateID();
      _srcTex.reserve(w, h, TextureSpecification("rgb=8"));
      _srcTex.bind(GL_TEXTURE0);
      glTexParameteri(_srcTex.textureTarget(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(_srcTex.textureTarget(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      _destBuffer.allocate(w, h);

      glGenTextures(1, &_lut1DTexID);

      if (radialLUTShader == 0)
      {
         radialLUTShader = new Cg_FragmentProgram("ParametricUndistortionFilter::shader");
         radialLUTShader->setProgramFromFile("undistort_radial_lut.cg");
#if 0
         char const * cgArgs[] = { "-bestprecision", "-nofastmath", "-nofastprecision", 0 };
         radialLUTShader->compile(cgArgs);
#else
         radialLUTShader->compile();
#endif
         checkGLErrorsHere0();
      }
   } // end Lookup1D_UndistortionFilter::allocate()

   void
   Lookup1D_UndistortionFilter::deallocate()
   {
      _srcTex.deallocateID();
      _destBuffer.deallocate();
      glDeleteTextures(1, &_lut1DTexID);
   }

   void
   Lookup1D_UndistortionFilter::setLookupTable(float pixelSize, int const lutSize, float const radialOffsets[])
   {
      _pixelSize = pixelSize;
      _lutSize   = lutSize;

      glBindTexture(GL_TEXTURE_1D, _lut1DTexID);
      glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE16F_ARB, lutSize, 0, GL_LUMINANCE, GL_FLOAT, radialOffsets);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   }

   void Lookup1D_UndistortionFilter::setRectifyingHomography(float const T_rect[9])
   {	    
		for(int i=0; i < 9; i++)
			_T_rect[i] = T_rect[i];
   }
   
   void applyT(float T[9], float x, float y, float &Tx, float&Ty, float &Tz)
   {
	  Tx = T[0]*x + T[1]*y + T[2];
	  Ty = T[3]*x + T[4]*y + T[5];
	  Tz = T[6]*x + T[7]*y + T[8];
   }

   void Lookup1D_UndistortionFilter::undistortColorImage(unsigned char const * image, unsigned char * result)
   {
	  float xh, yh, zh;

      _srcTex.overwriteWith(image, 3);

      _destBuffer.activate();
      setupNormalizedProjection();

      float const lutScale = _pixelSize / _lutSize;

      radialLUTShader->parameter("lutScale", lutScale);
      radialLUTShader->parameter("center", _center[0], _center[1]);
      radialLUTShader->parameter("wh", _width, _height, 1.0f/_width, 1.0f/_height);

      _srcTex.enable(GL_TEXTURE0);
      glActiveTexture(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_1D, _lut1DTexID);
      glEnable(GL_TEXTURE_1D);

      radialLUTShader->enable();
      glBegin(GL_TRIANGLES);

	  applyT(_T_rect,0,0,xh,yh,zh);
      glMultiTexCoord3f(GL_TEXTURE0_ARB, xh, yh,zh);	  
      glVertex2f(0, 0);
	  applyT(_T_rect,2*_width,0,xh,yh,zh);
      glMultiTexCoord3f(GL_TEXTURE0_ARB,xh, yh,zh);
      glVertex2f(2, 0);
	  applyT(_T_rect,0,2*_height,xh,yh,zh);
      glMultiTexCoord3f(GL_TEXTURE0_ARB, xh,yh,zh);
      glVertex2f(0, 2);
      glEnd();
      radialLUTShader->disable();

      _srcTex.disable(GL_TEXTURE0);
      glActiveTexture(GL_TEXTURE1_ARB);
      glDisable(GL_TEXTURE_1D);

      glReadPixels(0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, result);
   } // Lookup1D_UndistortionFilter::undistortColorImage()

   void
   Lookup1D_UndistortionFilter::undistortIntensityImage(unsigned char const * image, unsigned char * result)
   {
      _srcTex.overwriteWith(image, 1);

      _destBuffer.activate();
      setupNormalizedProjection();

      float const lutScale = _pixelSize / _lutSize;

      radialLUTShader->parameter("lutScale", lutScale);
      radialLUTShader->parameter("center", _center[0], _center[1]);
      radialLUTShader->parameter("wh", _width, _height, 1.0f/_width, 1.0f/_height);

      _srcTex.enable(GL_TEXTURE0);
      glActiveTexture(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_1D, _lut1DTexID);
      glEnable(GL_TEXTURE_1D);

      radialLUTShader->enable();
      glBegin(GL_TRIANGLES);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 0);
      glVertex2f(0, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 2*_width, 0);
      glVertex2f(2, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 2*_height);
      glVertex2f(0, 2);
      glEnd();
      radialLUTShader->disable();

      _srcTex.disable(GL_TEXTURE0);
      glActiveTexture(GL_TEXTURE1_ARB);
      glDisable(GL_TEXTURE_1D);

      glReadPixels(0, 0, _width, _height, GL_RED, GL_UNSIGNED_BYTE, result);
   } // end Lookup1D_UndistortionFilter::undistortIntensityImage()

} // end namespace V3D_GPU

#endif // defined(V3DLIB_GPGPU_ENABLE_CG)
