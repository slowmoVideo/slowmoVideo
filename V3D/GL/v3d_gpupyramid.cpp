#include "config.h"

#if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "v3d_gpupyramid.h"
#include "Base/v3d_timer.h"
#include "glsl_shaders.h"
#include <GL/glew.h>

#include <iostream>
#include <iomanip>
#include <cstdio>

using namespace std;
using namespace V3D_GPU;
using namespace V3D; 

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
	//         bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid buffer");
      }

    for (int level = 0; level < nLevels-1; ++level)
      {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmpFbIDs[level]);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _tmpTexID, level);
	//         bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid tmp buffer");
      }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmp2FbID);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _tmpTex2ID, 0);
    //      bool status = checkFrameBufferStatus(__FILE__, __LINE__, "pyramid tmp2 buffer");

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
#ifdef INCLUDE_SOURCE
	pass1HorizShader->setProgram(GlShaderStrings::pyramid_pass1v.c_str());
#else
	pass1HorizShader->setProgramFromFile("pyramid_pass1v.cg");
#endif
	pass1HorizShader->compile();
	checkGLErrorsHere0();
      } // end if (pass1HorizShader == 0)

    if (pass1VertShader == 0)
      {
	pass1VertShader = new Cg_FragmentProgram("PyramidCreator::buildPyramidForGrayscaleImage::pass1VertShader");
#ifdef INCLUDE_SOURCE
	pass1VertShader->setProgram(GlShaderStrings::pyramid_pass1h.c_str());
#else
	pass1VertShader->setProgramFromFile("pyramid_pass1h.cg");
#endif
	pass1VertShader->compile();
	checkGLErrorsHere0();
      } // end if (pass1VertShader == 0)

    if (pass2Shader == 0)
      {
	pass2Shader = new Cg_FragmentProgram("PyramidCreator::buildPyramidForGrayscaleImage::pass2Shader");
#ifdef INCLUDE_SOURCE
	pass2Shader->setProgram(GlShaderStrings::pyramid_pass2.c_str());
#else
	pass2Shader->setProgramFromFile("pyramid_pass2.cg");
#endif
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
    //      GLenum const textureTarget = GL_TEXTURE_2D;

    //    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbID);
    //    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _pyrTexID, level);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs[level]);
    glViewport(0, 0, W, H);
  }

  //----------------------------------------------------------------------

  void
  PyramidWithDerivativesCreator::allocate(int w, int h, int nLevels, int preSmoothingFilter)
  {
    ScopedTimer st(__PRETTY_FUNCTION__); 
    _width = w;
    _height = h;
    _nLevels = nLevels;
      _preSmoothingFilter = preSmoothingFilter;

    GLenum const textureTarget = GL_TEXTURE_2D;
    GLenum const floatFormat = _useFP32 ? GL_RGBA32F_ARB : GL_RGBA16F_ARB;

    glGenFramebuffersEXT(1, &_pyrFbIDs);
    glGenFramebuffersEXT(1, &_tmpFbIDs);
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
    glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0); 
    glTexParameteri(textureTarget, GL_TEXTURE_MAX_LEVEL, nLevels-1); 
    // This is the simplest way to create the full mipmap pyramid.
    for (int n = 0; n < nLevels; ++n)
      glTexImage2D(textureTarget, n, floatFormat, w >> n , h >> n , 0, GL_RGBA, GL_UNSIGNED_BYTE,0);
    checkGLErrorsHere0();

    //cout << "tmpTexID" << endl;
    glBindTexture(textureTarget, _tmpTexID);
    glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0); 
    glTexParameteri(textureTarget, GL_TEXTURE_MAX_LEVEL, nLevels-1);
    for (int n = 0; n < nLevels; ++n)
      glTexImage2D(textureTarget, n, floatFormat, w >> n, (h/2) >> n, 0, GL_RGBA, GL_UNSIGNED_BYTE,0);

    //cout << "tmpTex2ID" << endl;
    glBindTexture(textureTarget, _tmpTex2ID);
    glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
    glTexImage2D(textureTarget, 0, floatFormat, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindTexture(textureTarget, 0);
    checkGLErrorsHere0();

    initializeShaders(preSmoothingFilter); 
  } // end PyramidWithDerivativesCreator::allocate()

  void
  PyramidWithDerivativesCreator::initializeShaders(int preSmoothingFilter)
  {
    ScopedTimer st(__PRETTY_FUNCTION__); 
    if (shaders_initialized)
      return; 
     
    shaders_initialized = true; 

    if (_pass1HorizShader == 0)
      {
	_pass1HorizShader
            = new GLSL_FragmentProgram("PyramidWithDerivativesCreator::buildPyramidForGrayscaleImage_impl::pass1HorizShader");
         _pass1HorizShader->setProgram(GLSL_Shaders::pyramid_with_derivative_pass1v.c_str());
         _pass1HorizShader->compile();
	checkGLErrorsHere0();
      } // end if (_pass1HorizShader == 0)

    if (_pass1VertShader == 0)
      {
	_pass1VertShader
            = new GLSL_FragmentProgram("PyramidWithDerivativesCreator::buildPyramidForGrayscaleImage_impl::pass1VertShader");
         _pass1VertShader->setProgram(GLSL_Shaders::pyramid_with_derivative_pass1h.c_str());
         _pass1VertShader->compile();
	checkGLErrorsHere0();
      } // end if (_pass1VertShader == 0)

    if (_pass2Shader == 0)
      {
	_pass2Shader
            = new GLSL_FragmentProgram("PyramidWithDerivativesCreator::buildPyramidForGrayscaleImage_impl::pass2Shader");
         _pass2Shader->setProgram(GLSL_Shaders::pyramid_with_derivative_pass2.c_str());
	_pass2Shader->compile();
	checkGLErrorsHere0();
      } // end if (_pass2Shader == 0)
  }

  void
  PyramidWithDerivativesCreator::deallocate()
  {
    glDeleteFramebuffersEXT(1, &_pyrFbIDs);
    glDeleteFramebuffersEXT(1, &_tmpFbIDs);
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

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(textureTarget, srcTexID);
 
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmp2FbID);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureTarget, _tmpTex2ID, 0);

      _pass1HorizShader->parameter("presmoothing", _preSmoothingFilter);
    _pass1HorizShader->enable();
      _pass1HorizShader->bindTexture("src_tex", 0);
    renderQuad8Tap(0.0f, 1.0f/_height);
    _pass1HorizShader->disable();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,_pyrTexID,0);
    glBindTexture(textureTarget, _tmpTex2ID);

      _pass1VertShader->parameter("presmoothing", _preSmoothingFilter);
    _pass1VertShader->enable();
      _pass1VertShader->bindTexture("src_tex", 0);
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

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tmpFbIDs);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,_tmpTexID,level-1);
	glViewport(0, 0, W, H/2);
	renderQuad4Tap(0.0f, 1.0f/H);
	//glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,_pyrTexID,level);
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

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _pyrFbIDs);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, _pyrTexID, level);
    glViewport(0, 0, W, H);
  }

} // end namespace V3D_GPU

#endif // defined(V3DLIB_GPGPU_ENABLE_CG)
