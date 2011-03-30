#if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "v3d_gpupyramid.h"

#include <GL/glew.h>

#include <iostream>
#include <iomanip>
#include <cstdio>

using namespace std;
using namespace V3D_GPU;

namespace
{

   inline void
   renderQuad4Tap(float dS, float dT)
   {
      glBegin(GL_TRIANGLES);
      glMultiTexCoord4f(GL_TEXTURE0_ARB, 0-1*dS, 0-1*dT, 0-0*dS, 0-0*dT);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 0+1*dS, 0+1*dT, 0+2*dS, 0+2*dT);
      glVertex2f(0, 0);

      glMultiTexCoord4f(GL_TEXTURE0_ARB, 2-1*dS, 0-1*dT, 2-0*dS, 0-0*dT);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 2+1*dS, 0+1*dT, 2+2*dS, 0+2*dT);
      glVertex2f(2, 0);

      glMultiTexCoord4f(GL_TEXTURE0_ARB, 0-1*dS, 2-1*dT, 0-0*dS, 2-0*dT);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 0+1*dS, 2+1*dT, 0+2*dS, 2+2*dT);
      glVertex2f(0, 2);
      glEnd();
   } // end renderQuad()

   inline void
   renderQuad8Tap(float dS, float dT)
   {
      glBegin(GL_TRIANGLES);
      glMultiTexCoord4f(GL_TEXTURE0_ARB, 0-3*dS, 0-3*dT, 0-2*dS, 0-2*dT);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 0-1*dS, 0-1*dT, 0-0*dS, 0-0*dT);
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 0+1*dS, 0+1*dT, 0+2*dS, 0+2*dT);
      glMultiTexCoord4f(GL_TEXTURE3_ARB, 0+3*dS, 0+3*dT, 0+4*dS, 0+4*dT);
      glVertex2f(0, 0);

      glMultiTexCoord4f(GL_TEXTURE0_ARB, 2-3*dS, 0-3*dT, 2-2*dS, 0-2*dT);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 2-1*dS, 0-1*dT, 2-0*dS, 0-0*dT);
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 2+1*dS, 0+1*dT, 2+2*dS, 0+2*dT);
      glMultiTexCoord4f(GL_TEXTURE3_ARB, 2+3*dS, 0+3*dT, 2+4*dS, 0+4*dT);
      glVertex2f(2, 0);

      glMultiTexCoord4f(GL_TEXTURE0_ARB, 0-3*dS, 2-3*dT, 0-2*dS, 2-2*dT);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 0-1*dS, 2-1*dT, 0-0*dS, 2-0*dT);
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 0+1*dS, 2+1*dT, 0+2*dS, 2+2*dT);
      glMultiTexCoord4f(GL_TEXTURE3_ARB, 0+3*dS, 2+3*dT, 0+4*dS, 2+4*dT);
      glVertex2f(0, 2);
      glEnd();
   } // end renderQuad()

} // end namespace <>

//----------------------------------------------------------------------

namespace V3D_GPU
{

   void
   PyramidCreator::allocate(int w, int h, int nLevels)
   {
      _width = w;
      _height = h;
      _nLevels = nLevels;

      GLenum const textureTarget = GL_TEXTURE_2D;

      glGenFramebuffersEXT(nLevels, _pyrFbIDs);
      glGenFramebuffersEXT(nLevels, _tmpFbIDs);
      glGenFramebuffersEXT(1, &_tmp2FbID);

      glGenTextures(1, &_srcTexID);
      glGenTextures(1, &_pyrTexID);
      glGenTextures(1, &_tmpTexID);
      glGenTextures(1, &_tmpTex2ID);

      glBindTexture(textureTarget, _srcTexID);
      glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
      glTexImage2D(textureTarget, 0, GL_LUMINANCE8, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

      glBindTexture(textureTarget, _pyrTexID);
      glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      //glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
      for (int level = 0; level < nLevels; ++level)
      {
         int const W = (w >> level);
         int const H = (h >> level);
         glTexImage2D(textureTarget, level, GL_RGBA8, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
      }

      checkGLErrorsHere0();

      glBindTexture(textureTarget, _tmpTexID);
      glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      //glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
      for (int level = 0; level < nLevels-1; ++level)
      {
         int const W = (w >> level);
         int const H = (h >> level) / 2;
         glTexImage2D(textureTarget, level, GL_RGBA8, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
      }

      glBindTexture(textureTarget, _tmpTex2ID);
      glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
      glTexImage2D(textureTarget, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

      glBindTexture(textureTarget, 0);
      checkGLErrorsHere0();

      for (int level = 0; level < nLevels; ++level)
      {
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[level]);
         glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _pyrTexID, level);
         bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid buffer");
      }

      for (int level = 0; level < nLevels-1; ++level)
      {
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmpFbIDs[level]);
         glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _tmpTexID, level);
         bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid tmp buffer");
      }

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmp2FbID);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _tmpTex2ID, 0);
      bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid tmp2 buffer");

      checkGLErrorsHere0();
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
   } // end PyramidCreator::allocate()

   void
   PyramidCreator::deallocate()
   {
      glDeleteFramebuffersEXT(_nLevels, _pyrFbIDs);
      glDeleteFramebuffersEXT(_nLevels, _tmpFbIDs);
      glDeleteFramebuffersEXT(1, &_tmp2FbID);
      glDeleteTextures(1, &_pyrTexID);
      glDeleteTextures(1, &_srcTexID);
      glDeleteTextures(1, &_tmpTexID);
      glDeleteTextures(1, &_tmpTex2ID);
   }

   void
   PyramidCreator::buildPyramidForGrayscaleImage(uchar const * image)
   {
      static Cg_FragmentProgram * pass1HorizShader = 0;
      static Cg_FragmentProgram * pass1VertShader = 0;

      static Cg_FragmentProgram * pass2Shader = 0;

      if (pass1HorizShader == 0)
      {
         pass1HorizShader = new Cg_FragmentProgram("PyramidCreator::buildPyramidForGrayscaleImage::pass1HorizShader");
         pass1HorizShader->setProgramFromFile("pyramid_pass1v.cg");
         pass1HorizShader->compile();
         checkGLErrorsHere0();
      } // end if (pass1HorizShader == 0)

      if (pass1VertShader == 0)
      {
         pass1VertShader = new Cg_FragmentProgram("PyramidCreator::buildPyramidForGrayscaleImage::pass1VertShader");
         pass1VertShader->setProgramFromFile("pyramid_pass1h.cg");
         pass1VertShader->compile();
         checkGLErrorsHere0();
      } // end if (pass1VertShader == 0)

      if (pass2Shader == 0)
      {
         pass2Shader = new Cg_FragmentProgram("PyramidCreator::buildPyramidForGrayscaleImage::pass2Shader");
         pass2Shader->setProgramFromFile("pyramid_pass2.cg");
         pass2Shader->compile();
         checkGLErrorsHere0();
      } // end if (pass2Shader == 0)

      setupNormalizedProjection();
      glViewport(0, 0, _width, _height);

      GLenum const textureTarget = GL_TEXTURE_2D;

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(textureTarget, _srcTexID);
      glTexSubImage2D(textureTarget, 0, 0, 0, _width, _height, GL_LUMINANCE, GL_UNSIGNED_BYTE, image);

      glEnable(GL_TEXTURE_2D);

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmp2FbID);

      pass1HorizShader->enable();
      renderQuad8Tap(0.0f, 1.0f/_height);
      pass1HorizShader->disable();

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[0]);
      glBindTexture(textureTarget, _tmpTex2ID);

      pass1VertShader->enable();
      renderQuad8Tap(1.0f/_width, 0.0f);
      pass1VertShader->disable();

      pass2Shader->enable();
      pass2Shader->parameter("lod", 0);

      checkGLErrorsHere0();

      for (int level = 1; level < _nLevels; ++level)
      {
         // Source texture dimensions.
         int const W = (_width >> (level-1));
         int const H = (_height >> (level-1));

         //glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _tmpTexID, level-1);
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmpFbIDs[level-1]);
         glViewport(0, 0, W, H/2);

         glBindTexture(textureTarget, _pyrTexID);
         glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, level-1);
         renderQuad4Tap(0.0f, 1.0f/H);
         // Note: reset the base level to avoid odd GL errors.
         glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);

         glBindTexture(textureTarget, 0);
         glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _pyrTexID, level);
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[level]);
         glViewport(0, 0, W/2, H/2);

         glBindTexture(textureTarget, _tmpTexID);
         glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, level-1);
         renderQuad4Tap(1.0f/W, 0.0f);
         glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);

         glBindTexture(textureTarget, 0);
      } // end for (level)

      pass2Shader->disable();
      glDisable(GL_TEXTURE_2D);
   } // end PyramidCreator::buildPyramidForGrayscaleImage()

   void
   PyramidCreator::activateTarget(int level)
   {
      int const W = (_width >> level);
      int const H = (_height >> level);
      GLenum const textureTarget = GL_TEXTURE_2D;

//    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbID);
//    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _pyrTexID, level);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[level]);
      glViewport(0, 0, W, H);
   }

//----------------------------------------------------------------------

   void
   PyramidWithDerivativesCreator::allocate(int w, int h, int nLevels, int preSmoothingFilter)
   {
      _width = w;
      _height = h;
      _nLevels = nLevels;

      // Buidling Mipmaps with uninitialized pixels requires some time at least on a 8800.
      // Hence, we provide some valid initial data.
      vector<unsigned char> zeroPixels(3*w*h, 0);

      GLenum const textureTarget = GL_TEXTURE_2D;
      GLenum const floatFormat = _useFP32 ? GL_RGB32F_ARB : GL_RGB16F_ARB;

      glGenFramebuffersEXT(nLevels, _pyrFbIDs);
      glGenFramebuffersEXT(nLevels, _tmpFbIDs);
      glGenFramebuffersEXT(1, &_tmp2FbID);
      checkGLErrorsHere0();

      _srcTex.allocateID();
      _srcTex.reserve(w, h, TextureSpecification(_srcTexSpec));

      glGenTextures(1, &_pyrTexID);
      glGenTextures(1, &_tmpTexID);
      glGenTextures(1, &_tmpTex2ID);

      //cout << "pyrTexID" << endl;
      glBindTexture(textureTarget, _pyrTexID);
      glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      //glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      // This is the simplest way to create the full mipmap pyramid.
      glTexParameteri(textureTarget, GL_GENERATE_MIPMAP, GL_TRUE);
      glTexImage2D(textureTarget, 0, floatFormat, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, &zeroPixels[0]);
      glTexParameteri(textureTarget, GL_GENERATE_MIPMAP, GL_FALSE); 

      checkGLErrorsHere0();

      //cout << "tmpTexID" << endl;
      glBindTexture(textureTarget, _tmpTexID);
      glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      //glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(textureTarget, GL_GENERATE_MIPMAP, GL_TRUE);
      glTexImage2D(textureTarget, 0, floatFormat, w, h/2, 0, GL_RGB, GL_UNSIGNED_BYTE, &zeroPixels[0]);
      glTexParameteri(textureTarget, GL_GENERATE_MIPMAP, GL_FALSE); 

      //cout << "tmpTex2ID" << endl;
      glBindTexture(textureTarget, _tmpTex2ID);
      glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
      glTexImage2D(textureTarget, 0, GL_RGBA16F_ARB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

      glBindTexture(textureTarget, 0);
      checkGLErrorsHere0();

      //cout << "FBO allocation." << endl;
      for (int level = 0; level < nLevels; ++level)
      {
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[level]);
         glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _pyrTexID, level);
         bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid buffer");
      }

      for (int level = 0; level < nLevels-1; ++level)
      {
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmpFbIDs[level]);
         glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _tmpTexID, level);
         bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid tmp buffer");
      }

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmp2FbID);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _tmpTex2ID, 0);
      bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid tmp2 buffer");

      checkGLErrorsHere0();
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      vector<string> args;
      char str[512];
      sprintf(str, "-DPRESMOOTHING=%i", preSmoothingFilter);
      args.push_back(str);

      if (_pass1HorizShader == 0)
      {
         _pass1HorizShader
            = new Cg_FragmentProgram("PyramidWithDerivativesCreator::buildPyramidForGrayscaleImage_impl::pass1HorizShader");
         _pass1HorizShader->setProgramFromFile("pyramid_with_derivative_pass1v.cg");
         _pass1HorizShader->compile(args);
         checkGLErrorsHere0();
      } // end if (_pass1HorizShader == 0)

      if (_pass1VertShader == 0)
      {
         _pass1VertShader
            = new Cg_FragmentProgram("PyramidWithDerivativesCreator::buildPyramidForGrayscaleImage_impl::pass1VertShader");
         _pass1VertShader->setProgramFromFile("pyramid_with_derivative_pass1h.cg");
         _pass1VertShader->compile(args);
         checkGLErrorsHere0();
      } // end if (_pass1VertShader == 0)

      if (_pass2Shader == 0)
      {
         _pass2Shader
            = new Cg_FragmentProgram("PyramidWithDerivativesCreator::buildPyramidForGrayscaleImage_impl::pass2Shader");
         _pass2Shader->setProgramFromFile("pyramid_with_derivative_pass2.cg");
         _pass2Shader->compile();
         checkGLErrorsHere0();
      } // end if (_pass2Shader == 0)
   } // end PyramidWithDerivativesCreator::allocate()

   void
   PyramidWithDerivativesCreator::deallocate()
   {
      glDeleteFramebuffersEXT(_nLevels, _pyrFbIDs);
      glDeleteFramebuffersEXT(_nLevels, _tmpFbIDs);
      glDeleteFramebuffersEXT(1, &_tmp2FbID);
      glDeleteTextures(1, &_pyrTexID);
      glDeleteTextures(1, &_tmpTexID);
      glDeleteTextures(1, &_tmpTex2ID);
      _srcTex.deallocateID();
   }

   void
   PyramidWithDerivativesCreator::buildPyramidForGrayscaleImage(uchar const * image)
   {
      _srcTex.overwriteWith(image, 1);
      this->buildPyramidForGrayscaleTexture(_srcTex.textureID());
   }

   void
   PyramidWithDerivativesCreator::buildPyramidForGrayscaleImage(float const * image)
   {
      _srcTex.overwriteWith(image, 1);
      this->buildPyramidForGrayscaleTexture(_srcTex.textureID());
   }

   void
   PyramidWithDerivativesCreator::buildPyramidForGrayscaleTexture(unsigned int srcTexID)
   {
      setupNormalizedProjection();
      glViewport(0, 0, _width, _height);

      GLenum const textureTarget = GL_TEXTURE_2D;

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(textureTarget, srcTexID);
      glEnable(GL_TEXTURE_2D);

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmp2FbID);

      _pass1HorizShader->enable();
      renderQuad8Tap(0.0f, 1.0f/_height);
      _pass1HorizShader->disable();

      //glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _pyrTexID, 0);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[0]);
      glBindTexture(textureTarget, _tmpTex2ID);

      _pass1VertShader->enable();
      renderQuad8Tap(1.0f/_width, 0.0f);
      _pass1VertShader->disable();

      _pass2Shader->enable();

      for (int level = 1; level < _nLevels; ++level)
      {
         // Source texture dimensions.
         int const W = (_width >> (level-1));
         int const H = (_height >> (level-1));

         glBindTexture(textureTarget, _pyrTexID);
         glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, level-1);

         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmpFbIDs[level-1]);
         glViewport(0, 0, W, H/2);
         renderQuad4Tap(0.0f, 1.0f/H);
         //glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);

         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[level]);
         glViewport(0, 0, W/2, H/2);
         glBindTexture(textureTarget, _tmpTexID);
         glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, level-1);
         renderQuad4Tap(1.0f/W, 0.0f);
         //glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
      } // end for (level)

      _pass2Shader->disable();
      glDisable(GL_TEXTURE_2D);

      glBindTexture(textureTarget, _pyrTexID);
      glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
      glBindTexture(textureTarget, _tmpTexID);
      glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
   } // end PyramidWithDerivativesCreator::buildPyramidForGrayscaleTexturel()

   void
   PyramidWithDerivativesCreator::activateTarget(int level)
   {
      int const W = (_width >> level);
      int const H = (_height >> level);
      GLenum const textureTarget = GL_TEXTURE_2D;

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[level]);
      glViewport(0, 0, W, H);
   }

} // end namespace V3D_GPU

#endif // defined(V3DLIB_GPGPU_ENABLE_CG)
