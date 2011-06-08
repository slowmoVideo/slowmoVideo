#if defined(V3DLIB_ENABLE_GPGPU)

#include "v3d_gpubase.h"
#include "Base/v3d_exception.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#if defined(V3DLIB_GPGPU_ENABLE_CG)
# include <Cg/cg.h>
# include <Cg/cgGL.h>
#endif

namespace
{

   unsigned int const extPixelFormat[] = { 0, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };

   inline int
   intFromString(std::string const& value, int defaultVal)
   {
      if (value.empty()) return defaultVal;
      return atoi(value.c_str());
   } // end intFromString()

   template <typename T>
   inline void
   interleavePixels(int const w, int const h, T const * red, T const * green, T const * blue, T * pixels)
   {
      for (int p = 0; p < w*h; ++p)
      {
         pixels[3*p+0] = red[p];
         pixels[3*p+1] = green[p];
         pixels[3*p+2] = blue[p];
      }
   } // end interleavePixels()

   template <typename T>
   inline void
   interleavePixels(int const w, int const h, T const * red, T const * green, T const * blue, T const * alpha,
                    T * pixels)
   {
      for (int p = 0; p < w*h; ++p)
      {
         pixels[4*p+0] = red[p];
         pixels[4*p+1] = green[p];
         pixels[4*p+2] = blue[p];
         pixels[4*p+3] = alpha[p];
      }
   } // end interleavePixels()

} // end namespace <>

namespace V3D_GPU
{

   using namespace std;

   TextureSpecification::TextureSpecification(char const * specString)
      : nChannels(3), nBitsPerChannel(8), isFloatTexture(false), isDepthTexture(false),
        isRTT(true), enableTextureRG(false)
   {
      istringstream is(specString);

      string token, key, value;

      while (!is.eof())
      {
         is >> token;

         string::size_type pos = 0;

         if ((pos = token.find("=")) != token.npos)
         {
            key = token.substr(0, pos);
            value = token.substr(pos+1, token.length()-pos+1);
         }
         else
         {
            key = token;
            value = "";
         }

         if (key == "r")
         {
            this->nChannels = 1;
            if (value.find("f") != value.npos)
               this->isFloatTexture = true;
            this->nBitsPerChannel = intFromString(value, this->isFloatTexture ? 32 : 8);
         }
         else if (key == "rg")
         {
            this->nChannels = 2;
            if (value.find("f") != value.npos)
               this->isFloatTexture = true;
            this->nBitsPerChannel = intFromString(value, this->isFloatTexture ? 32 : 8);
         }
         else if (key == "rgb")
         {
            this->nChannels = 3;
            if (value.find("f") != value.npos)
               this->isFloatTexture = true;
            this->nBitsPerChannel = intFromString(value, this->isFloatTexture ? 32 : 8);
         }
         else if (key == "rgba")
         {
            this->nChannels = 4;
            if (value.find("f") != value.npos)
               this->isFloatTexture = true;
            this->nBitsPerChannel = intFromString(value, this->isFloatTexture ? 32 : 8);
         }
         else if (key == "depth")
         {
            this->isDepthTexture = true;
            this->nBitsPerChannel = intFromString(value, 24);
         }
         else if (key == "RTT")
         {
            this->isRTT = true;
         }
         else if (key == "noRTT")
         {
            this->isRTT = false;
         }
         else if (key == "enableTextureRG")
         {
#if defined(GL_ARB_texture_rg)
            this->enableTextureRG = true;
#endif
         }
         else if (key == "tex2D")
         {
            // Ignore that keyword
         }
         else
         {
            cerr << "TextureSpecification::TextureSpecification(): "
                 << "Warning Unknown keyword: '" << key << "'; ignored." << endl;
         }
      } // end while

      if ((this->nChannels < 3) && this->isRTT && !this->enableTextureRG)
      {
         cerr << "TextureSpecification::TextureSpecification(): "
              << "Warning: luminance or luminance/alpha texture will not work as render target." << endl;
      }
   } // end TextureSpecification::TextureSpecification()

   unsigned int
   TextureSpecification::getGLInternalFormat() const
   {
      if (!this->isDepthTexture)
      {
         if (!this->enableTextureRG)
         {
            if (!this->isFloatTexture)
            {
               if (this->nBitsPerChannel == 8)
               {
                  unsigned int const formats[5] = { 0,
                                                    GL_LUMINANCE8,
                                                    GL_LUMINANCE8_ALPHA8,
                                                    GL_RGB8,
                                                    GL_RGBA8 };
                  return formats[this->nChannels];
               }
               else if (this->nBitsPerChannel == 16)
               {
                  unsigned int const formats[5] = { 0,
                                                    GL_LUMINANCE16,
                                                    GL_LUMINANCE16_ALPHA16,
                                                    GL_RGB16,
                                                    GL_RGBA16 };
                  return formats[this->nChannels];
               }
               else
               {
                  raiseGLErrorHere1("Unsupported number of bits for int texture (8 or 16 bits).");
               }
            }
            else
            {
               if (this->nBitsPerChannel == 32)
               {
                  unsigned int const formats[5] = { 0,
                                                    GL_LUMINANCE32F_ARB,
                                                    GL_LUMINANCE_ALPHA32F_ARB,
                                                    GL_RGB32F_ARB,
                                                    GL_RGBA32F_ARB };
                  return formats[this->nChannels];
               }
               else if (this->nBitsPerChannel == 16)
               {
                  unsigned int const formats[5] = { 0,
                                                    GL_LUMINANCE16F_ARB,
                                                    GL_LUMINANCE_ALPHA16F_ARB,
                                                    GL_RGB16F_ARB,
                                                    GL_RGBA16F_ARB };
                  return formats[this->nChannels];
               }
               else
               {
                  raiseGLErrorHere1("Unsupported number of bits for float texture (16 or 32 bits).");
               }
            } // end if (!useFloat)
         }
         else
         {
#if defined(GL_ARB_texture_rg)
            // Note: this requires the ARB_texture_rg extension
            if (!this->isFloatTexture)
            {
               if (this->nBitsPerChannel == 8)
               {
                  unsigned int const formats[5] = { 0,
                                                    GL_R8,
                                                    GL_RG8,
                                                    GL_RGB8,
                                                    GL_RGBA8 };
                  return formats[this->nChannels];
               }
               else if (this->nBitsPerChannel == 16)
               {
                  unsigned int const formats[5] = { 0,
                                                    GL_R16,
                                                    GL_RG16,
                                                    GL_RGB16,
                                                    GL_RGBA16 };
                  return formats[this->nChannels];
               }
               else
               {
                  raiseGLErrorHere1("Unsupported number of bits for int texture (8 or 16 bits).");
               }
            }
            else
            {
               if (this->nBitsPerChannel == 32)
               {
                  unsigned int const formats[5] = { 0,
                                                    GL_R32F,
                                                    GL_RG32F,
                                                    GL_RGB32F_ARB,
                                                    GL_RGBA32F_ARB };
                  return formats[this->nChannels];
               }
               else if (this->nBitsPerChannel == 16)
               {
                  unsigned int const formats[5] = { 0,
                                                    GL_R16F,
                                                    GL_RG16F,
                                                    GL_RGB16F_ARB,
                                                    GL_RGBA16F_ARB };
                  return formats[this->nChannels];
               }
               else
               {
                  raiseGLErrorHere1("Unsupported number of bits for float texture (16 or 32 bits).");
               }
            } // end if (!useFloat)
#else
            raiseGLErrorHere1("Attempting to use ARB_texture_rg, but support is not compiled in.");
#endif
         } // end if ((!this->enableTextureRG)
      }
      else
      {
         switch (nBitsPerChannel)
         {
            case 16:
               return GL_DEPTH_COMPONENT16_ARB;
            case 24:
               return GL_DEPTH_COMPONENT24_ARB;
            case 32:
               return GL_DEPTH_COMPONENT32_ARB;
            default:
               raiseGLErrorHere1("Unsupported number of bits for depth texture (16, 24 or 32 bits).");
         }
      } // end if (!isDepthTexture)
      return 0;
   } // end TextureSpecification::getGLInternalFormat()

//----------------------------------------------------------------------

   bool
   ImageTexture2D::allocateID()
   {
      _textureTarget = GL_TEXTURE_2D;
      glGenTextures(1, &_textureID);

      glBindTexture(_textureTarget, _textureID);

      // Default is clamp to edge and nearest filtering.
      glTexParameteri(_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
      glTexParameteri(_textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(_textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      checkGLErrorsHere1(_texName);
      return true;
   }

   void
   ImageTexture2D::deallocateID()
   {
      if (_textureID == 0) return;
      this->clear();
      glDeleteTextures(1, &_textureID);
      _textureID = 0;
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::reserve(int width, int height, TextureSpecification const& texSpec)
   {
      _width  = width;
      _height = height;

      unsigned internalFormat = texSpec.getGLInternalFormat();
      unsigned format = texSpec.isDepthTexture ? GL_DEPTH_COMPONENT : GL_RGB;
      unsigned type = texSpec.isFloatTexture ? GL_FLOAT : GL_UNSIGNED_BYTE;

      glBindTexture(_textureTarget, _textureID);
      glTexImage2D(_textureTarget, 0, internalFormat, _width, _height, 0, format, type, 0);
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::overwriteWith(uchar const * pixels, int nChannels)
   {
      unsigned const format = extPixelFormat[nChannels];
      unsigned const type = GL_UNSIGNED_BYTE;

      glBindTexture(_textureTarget, _textureID);
      glTexSubImage2D(_textureTarget, 0, 0, 0, _width, _height, format, type, pixels);
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::overwriteWith(uchar const * redPixels, uchar const * greenPixels, uchar const * bluePixels)
   {
      unsigned const format = extPixelFormat[3];
      unsigned const type = GL_UNSIGNED_BYTE;

      unsigned char * pixels = new unsigned char[3 * _width * _height];

      interleavePixels(_width, _height, redPixels, greenPixels, bluePixels, pixels);
      glBindTexture(_textureTarget, _textureID);
      glTexSubImage2D(_textureTarget, 0, 0, 0, _width, _height, format, type, pixels);

      delete [] pixels;
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::overwriteWith(uchar const * redPixels, uchar const * greenPixels,
                                 uchar const * bluePixels, uchar const * alphaPixels)
   {
      unsigned const format = extPixelFormat[4];
      unsigned const type = GL_UNSIGNED_BYTE;

      unsigned char * pixels = new unsigned char[4 * _width * _height];

      interleavePixels(_width, _height, redPixels, greenPixels, bluePixels, alphaPixels, pixels);
      glBindTexture(_textureTarget, _textureID);
      glTexSubImage2D(_textureTarget, 0, 0, 0, _width, _height, format, type, pixels);

      delete [] pixels;
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::overwriteWith(float const * pixels, int nChannels)
   {
      unsigned const format = extPixelFormat[nChannels];
      unsigned const type = GL_FLOAT;

      glBindTexture(_textureTarget, _textureID);
      glTexSubImage2D(_textureTarget, 0, 0, 0, _width, _height, format, type, pixels);
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::overwriteWith(float const * redPixels, float const * greenPixels, float const * bluePixels)
   {
      unsigned const format = extPixelFormat[3];
      unsigned const type = GL_FLOAT;

      float * pixels = new float[3 * _width * _height];

      interleavePixels(_width, _height, redPixels, greenPixels, bluePixels, pixels);
      glBindTexture(_textureTarget, _textureID);
      glTexSubImage2D(_textureTarget, 0, 0, 0, _width, _height, format, type, pixels);

      delete [] pixels;
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::overwriteWith(float const * redPixels, float const * greenPixels,
                                 float const * bluePixels, float const * alphaPixels)
   {
      unsigned const format = extPixelFormat[4];
      unsigned const type = GL_FLOAT;

      float * pixels = new float[4 * _width * _height];

      interleavePixels(_width, _height, redPixels, greenPixels, bluePixels, alphaPixels, pixels);
      glBindTexture(_textureTarget, _textureID);
      glTexSubImage2D(_textureTarget, 0, 0, 0, _width, _height, format, type, pixels);

      delete [] pixels;
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::clear()
   {
      glBindTexture(_textureTarget, _textureID);
      glTexImage2D(_textureTarget, 0, GL_RGB, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      checkGLErrorsHere1(_texName);
   }

   void
   ImageTexture2D::bind()
   {
      glBindTexture(_textureTarget, _textureID);
   }

   void
   ImageTexture2D::bind(unsigned texUnit)
   {
      glActiveTexture(texUnit);
      this->bind();
   }

   void
   ImageTexture2D::enable()
   {
      glBindTexture(_textureTarget, _textureID);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glEnable(_textureTarget);
   }

   void
   ImageTexture2D::enable(unsigned texUnit)
   {
      glActiveTexture(texUnit);
      this->enable();
   }

   void
   ImageTexture2D::disable()
   {
      glDisable(_textureTarget);
   }

   void
   ImageTexture2D::disable(unsigned texUnit)
   {
      glActiveTexture(texUnit);
      this->disable();
   }

//----------------------------------------------------------------------

   bool
   FrameBufferObject::allocate()
   {
      glGenFramebuffersEXT(1, &_fboID);
      checkGLErrorsHere1(_fboName);
      return true;
   }

   void
   FrameBufferObject::deallocate()
   {
      std::fill(_attachedColorTextures, _attachedColorTextures+16, (ImageTexture2D *)0);
      glDeleteFramebuffersEXT(1, &_fboID);
      checkGLErrorsHere1(_fboName);
   }

   void
   FrameBufferObject::makeCurrent()
   {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fboID);
      checkGLErrorsHere1(_fboName);
   }

   void
   FrameBufferObject::activate(bool setViewport)
   {
      this->makeCurrent();
      if (this->checkValidity())
      {
         if (setViewport)
            glViewport(0, 0, this->width(), this->height());
      }
   }

   bool
   FrameBufferObject::isCurrent()
   {
      GLint curFboID;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &curFboID);
      return (_fboID == curFboID);
   }

   void
   FrameBufferObject::attachTexture2D(ImageTexture2D& texture, GLenum attachment, int mipLevel)
   {
      if (this->checkBinding("FrameBufferObject::attachTexture2D()"))
      {
         if (attachment >= GL_COLOR_ATTACHMENT0_EXT && attachment <= GL_COLOR_ATTACHMENT15_EXT)
         {
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment,
                                      texture.textureTarget(), texture.textureID(), mipLevel);
            _attachedColorTextures[attachment - GL_COLOR_ATTACHMENT0_EXT] = &texture;
         }
         else if (attachment == GL_DEPTH_ATTACHMENT_EXT)
         {
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment,
                                      texture.textureTarget(), texture.textureID(), mipLevel);
            _attachedDepthTexture = &texture;
         }
         else
            raiseGLErrorHere2("Unknown/unsupported attachment specifier", _fboName.c_str());
      }
      checkGLErrorsHere1(_fboName);
   }

   void
   FrameBufferObject::attachTextures2D(int numTextures, ImageTexture2D * textures,
                                       GLenum * attachment, int * mipLevel)
   {
      for (int i = 0; i < numTextures; ++i)
         this->attachTexture2D(textures[i],
                               (attachment != 0) ? attachment[i] : (GL_COLOR_ATTACHMENT0_EXT + i),
                               (mipLevel != 0) ? mipLevel[i] : 0);
   }

   void
   FrameBufferObject::detach(GLenum attachment)
   {
      if (this->checkBinding("FrameBufferObject::detach()"))
      {
         if (attachment >= GL_COLOR_ATTACHMENT0_EXT && attachment <= GL_COLOR_ATTACHMENT15_EXT)
         {
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_2D, 0, 0);
            _attachedColorTextures[attachment - GL_COLOR_ATTACHMENT0_EXT] = 0;
         }
         else if (attachment == GL_DEPTH_ATTACHMENT_EXT)
         {
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_2D, 0, 0);
            _attachedDepthTexture = 0;
         }
         else
            raiseGLErrorHere2("Unknown/unsupported attachment specifier", _fboName.c_str());
      }
      checkGLErrorsHere1(_fboName);
   }

   void
   FrameBufferObject::detachAll()
   {
      int const numAttachments = this->getMaxColorAttachments();
      for (int i = 0; i < numAttachments; ++i)
         this->detach(GL_COLOR_ATTACHMENT0_EXT + i);
   }

# ifndef NDEBUG_GL
   bool
   FrameBufferObject::checkValidity(std::ostream& os)
   {
      if (!this->checkBinding("FrameBufferObject::checkValidity()"))
         return false;

      GLenum const status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

      switch(status)
      {
         case GL_FRAMEBUFFER_COMPLETE_EXT: // Everything's OK
            return true;
         case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            raiseGLErrorHere2("Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT) ", _fboName.c_str());
            return false;
         case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            raiseGLErrorHere2("Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT) ", _fboName.c_str());
            return false;
         case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            raiseGLErrorHere2("Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT) ", _fboName.c_str());
            return false;
         case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            raiseGLErrorHere2("Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT) ", _fboName.c_str());
            return false;
         case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            raiseGLErrorHere2("Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT) ", _fboName.c_str());
            return false;
         case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            raiseGLErrorHere2("Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT) ", _fboName.c_str());
            return false;
         case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            raiseGLErrorHere2("Frame buffer is incomplete (GL_FRAMEBUFFER_UNSUPPORTED_EXT) ", _fboName.c_str());
            return false;
         default:
            raiseGLErrorHere2("Frame buffer is incomplete (unknown error code) ", _fboName.c_str());
            return false;
      }
      return false;
   }
# endif

   int
   FrameBufferObject::getMaxColorAttachments()
   {
      GLint res = 0;
      glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &res);
      return res;
   }

   void
   FrameBufferObject::disableFBORendering()
   {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
   }

   bool
   FrameBufferObject::checkBinding(char const * what)
   {
      GLint curFboID;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &curFboID);

      ostringstream oss;
      oss << "FBO operation (" << what << ") on unbound frame buffer attempted";

      if (curFboID != _fboID)
      {
         raiseGLErrorHere2(oss.str().c_str(), this->_fboName);
         return false;
      }
      return true;
   }

//----------------------------------------------------------------------

# ifndef NDEBUG_GL
   void
   checkGLErrors(char const * location, ostream& os) 
   {
      GLuint errnum;
      char const * errstr;
      bool hasError = false;
      while (errnum = glGetError())
      {
         errstr = reinterpret_cast<const char *>(gluErrorString(errnum));
         if (errstr)
            os << errstr; 
         else
            os << "Error " << errnum;
    
         os << " at " << location << endl;
#ifdef WIN32
		 break;
#endif
      }
      if(hasError)
         throwV3DErrorHere("");
   }

   void
   checkGLErrors(char const * file, int line, ostream& os)
   {
      GLuint errnum;
      char const * errstr;
      bool hasError = false;
      while (errnum = glGetError())
      {
         hasError = true;
         errstr = reinterpret_cast<const char *>(gluErrorString(errnum));
         if (errstr)
            os << errstr; 
         else
            os << "Error " << errnum;

         os << " at " << file << ":" << line << endl;
#ifdef WIN32
		 break;
#endif
      }
      if(hasError)
         throwV3DErrorHere("");
   }

   void
   checkGLErrors(char const * file, int line, string const& name, ostream& os)
   {
      GLuint errnum;
      char const * errstr;
      bool hasError = false;
      while (errnum = glGetError())
      {
         errstr = reinterpret_cast<const char *>(gluErrorString(errnum));
         if (errstr)
            os << errstr; 
         else
            os << "Error " << errnum;

         os << " with " << name << " at " << file << ":" << line << endl;
#ifdef WIN32
		 break;
#endif
      }
      if(hasError)
         throwV3DErrorHere("");
   }

   bool
   checkFrameBufferStatus(char const * file, int line, std::string const& name, std::ostream& os)
   {
      GLenum const status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

      char const * msg = 0;

      switch(status)
      {
         case GL_FRAMEBUFFER_COMPLETE_EXT: // Everything's OK
            return true;
         case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            msg = "Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT) ";
            break;
         case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            msg = "Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT) ";
            break;
         case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            msg = "Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT) ";
            break;
         case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            msg = "Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT) ";
            break;
         case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            msg = "Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT) ";
            break;
         case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            msg = "Frame buffer is incomplete (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT) ";
            break;
         case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            msg = "Frame buffer is incomplete (GL_FRAMEBUFFER_UNSUPPORTED_EXT) ";
            break;
         default:
            msg = "Frame buffer is incomplete (unknown error code) ";
            break;
      }
      os << msg << " with " << name << " at " << file << ":" << line << endl;
      return false;
   } // end checkFrameBufferStatus()

# endif

   void
   raiseGLError(char const * file, int line, char const * msg, std::ostream& os)
   {
      os << msg << " at " << file << ":" << line << endl;
   }

   void
   raiseGLError(char const * file, int line, char const * msg, std::string const& name, std::ostream& os)
   {
      os << msg << " with " << name << " at " << file << ":" << line << endl;
   }

} // end namespace V3D_GPU

//----------------------------------------------------------------------
// CG SHADER ROUTINES
//----------------------------------------------------------------------

namespace V3D_GPU
{

   void
   ProgramBase::setProgramFromFile(char const * fileName)
   {
      ostringstream oss;

      char completeName[1024];
      char completeName2[1024];

      if (fileName[0] == '/')
      {
         // Absolute path for the shader file
         strncpy(completeName, fileName, 1024);
      }
      else
      {
         char const * baseDir = getenv("V3D_SHADER_DIR");
         if (baseDir == 0)
            baseDir = "."; // If environment var is not set, use "."

#ifdef WIN32
         sprintf_s(completeName, "%s/%s", baseDir, fileName);
#else
         snprintf(completeName, 1024, "%s/%s", baseDir, fileName);
#endif
      }
      
      ifstream is(completeName);

      if (!is)
      {
#ifdef WIN32
         sprintf_s(completeName2, "./%s", fileName);
#else
         snprintf(completeName2, 1024, "./%s", fileName);
#endif
         is.open(completeName2);
      }

      char buf[1024];

      if (!is)
      {
         raiseGLErrorHere2("Cannot load shader file.", _shaderName.c_str());
         cerr << "Tried " << completeName << " and " << completeName2 << "." << endl;
         return;
      }

      while (!is.eof())
      {
         is.get(buf, 1024, -1);
         oss << buf;
      }

      is.close();

      this->setProgram(oss.str());
   } // end GPU_ProgramBase::setProgramFromFile()

} // end namespace V3D_GPU

# if defined(V3DLIB_GPGPU_ENABLE_CG)

namespace
{
   CGprofile fragmentProfile = CG_PROFILE_UNKNOWN;
   CGprofile vertexProfile   = CG_PROFILE_UNKNOWN;

} // end namespace

namespace V3D_GPU
{

   CGcontext Cg_ProgramBase::_context = 0;

   char const *
   Cg_ProgramBase::getCompiledString()
   {
      if (!cgIsProgramCompiled(_program)) return 0;
      return cgGetProgramString(_program, CG_COMPILED_PROGRAM);
   }

   void
   Cg_ProgramBase::initializeCg()
   {
      if (_context == 0)
      {
         cgSetErrorCallback(Cg_ProgramBase::handleCgError);
         _context = cgCreateContext();

         fragmentProfile = CG_PROFILE_UNKNOWN;
         vertexProfile   = CG_PROFILE_UNKNOWN;

         if (cgGLIsProfileSupported(CG_PROFILE_ARBFP1)) fragmentProfile = CG_PROFILE_ARBFP1;
         if (cgGLIsProfileSupported(CG_PROFILE_FP20))   fragmentProfile = CG_PROFILE_FP20;
         if (cgGLIsProfileSupported(CG_PROFILE_FP30))   fragmentProfile = CG_PROFILE_FP30;
         if (cgGLIsProfileSupported(CG_PROFILE_FP40))   fragmentProfile = CG_PROFILE_FP40;
         if (cgGLIsProfileSupported(CG_PROFILE_GPU_FP)) fragmentProfile = CG_PROFILE_GPU_FP;

         if (cgGLIsProfileSupported(CG_PROFILE_ARBVP1)) vertexProfile = CG_PROFILE_ARBVP1;
         if (cgGLIsProfileSupported(CG_PROFILE_VP20))   vertexProfile = CG_PROFILE_VP20;
         if (cgGLIsProfileSupported(CG_PROFILE_VP30))   vertexProfile = CG_PROFILE_VP30;
         if (cgGLIsProfileSupported(CG_PROFILE_VP40))   vertexProfile = CG_PROFILE_VP40;
         if (cgGLIsProfileSupported(CG_PROFILE_GPU_VP)) vertexProfile = CG_PROFILE_GPU_VP;

         if (fragmentProfile == CG_PROFILE_UNKNOWN)
            cerr << "Warning: No useful fragment shader profile found." << endl;
         if (vertexProfile == CG_PROFILE_UNKNOWN)
            cerr << "Warning: No useful vertex shader profile found." << endl;
      }
   }

   Cg_ProgramBase::~Cg_ProgramBase()
   {
      if (_program == 0) return;
      cgDestroyProgram(_program);
   }

   void
   Cg_ProgramBase::handleCgError() 
   {
      cerr << "Cg error: " << cgGetErrorString(cgGetError()) << endl;;
   }

//----------------------------------------------------------------------

   void
   Cg_FragmentProgram::setProgram(char const * source)
   {
      _source = source;
      if (_program != 0) cgDestroyProgram(_program);
      _program = 0;
   }

   void
   Cg_FragmentProgram::compile(char const * * compilerArgs, char const *entry)
   {
      if (_program == 0)
      {
         _program = cgCreateProgram(_context, CG_SOURCE, _source.c_str(),
                                    fragmentProfile, entry, compilerArgs);
         if (_program == 0)
         {
            cerr << "Program: " << _shaderName << endl;
            cerr << "Cg compiler report:" << endl << cgGetLastListing(_context) << endl;
         }
      }
      if (!cgIsProgramCompiled(_program))
         cgCompileProgram(_program);
   }

   void
   Cg_FragmentProgram::compile(std::vector<std::string> const& compilerArgs, char const *entry)
   {
      size_t const nArgs = compilerArgs.size();
      char const * * args = new char const* [nArgs + 1];
      for (size_t i = 0; i < nArgs; ++i)
         args[i] = compilerArgs[i].c_str();
      args[nArgs] = 0;
      this->compile(args,entry);
      delete [] args;
   }

   void
   Cg_FragmentProgram::enable()
   {
      cgGLEnableProfile(fragmentProfile);
      cgGLLoadProgram(_program);
      cgGLBindProgram(_program);
   }

   void 
   Cg_FragmentProgram::disable()
   {
      cgGLDisableProfile(fragmentProfile);
   }

   void
   Cg_FragmentProgram::parameter(char const * param, float x)
   {
      cgSetParameter1f(cgGetNamedParameter(_program, param), x);
   }

   void
   Cg_FragmentProgram::parameter(char const * param, float x, float y)
   {
      cgSetParameter2f(cgGetNamedParameter(_program, param), x, y);
   }

   void
   Cg_FragmentProgram::parameter(char const * param, float x, float y, float z)
   {
      cgSetParameter3f(cgGetNamedParameter(_program, param), x, y, z);
   }

   void
   Cg_FragmentProgram::parameter(char const * param, float x, float y, float z, float w)
   {
      cgSetParameter4f(cgGetNamedParameter(_program, param), x, y, z, w);
   }

   void
   Cg_FragmentProgram::parameter(char const * param, int len, float const * array)
   {
      cgGLSetParameterArray1f(cgGetNamedParameter(_program, param), 0, len, array);
   }

   void
   Cg_FragmentProgram::matrixParameterR(char const * param, int rows, int cols, double const * values)
   {
      if (rows > 4 || cols > 4)
      {
         raiseGLErrorHere2("Matrix parameter should be <= 4x4.", _shaderName.c_str());
         return;
      }

      cgGLSetMatrixParameterdr(cgGetNamedParameter(_program, param), values);
   }

   void
   Cg_FragmentProgram::matrixParameterC(char const * param, int rows, int cols, double const * values)
   {
      if (rows > 4 || cols > 4)
      {
         raiseGLErrorHere2("Matrix parameter should be <= 4x4.", _shaderName.c_str());
         return;
      }

      cgGLSetMatrixParameterdc(cgGetNamedParameter(_program, param), values);
   }

   unsigned
   Cg_FragmentProgram::getTexUnit(char const * param)
   {
      return cgGLGetTextureEnum(cgGetNamedParameter(_program, param));
   }

} // end namespace V3D_GPU

# endif // defined(V3DLIB_GPGPU_ENABLE_CG)

//----------------------------------------------------------------------

# if defined(V3DLIB_GPGPU_ENABLE_GLSL)

namespace V3D_GPU
{

   void
   GLSL_FragmentProgram::setProgram(char const * source)
   {
      _source = source;
      // To enforce recompilation.
      if (_program) glDeleteObjectARB(_program);
      _program = 0;
   }

   void
   GLSL_FragmentProgram::compile(char const * * compilerArgs, char const *entry = 0)
   {
      if (compilerArgs != 0)
         cerr << "GLSL_FragmentProgram::compile(): arguments to the compiler are not supported (and ignored)." << endl;
      if(entry != 0)
          cerr << "GLSL_FragmentProgram::compile(): named entry point is not supported (and ignored)." << endl;

      checkGLErrorsHere1(_shaderName);

      if (_program == 0)
      {
         int len = _source.length() + 1;
         GLcharARB * str = new char[len];
         strcpy(str, _source.c_str());

         GLint success = GL_FALSE;
         GLint logLength;

         _program = glCreateProgramObjectARB();

         unsigned fragmentProgram = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
         glShaderSourceARB(fragmentProgram, 1, (GLcharARB const * *)&str, &len);
         glCompileShaderARB(fragmentProgram);
         delete [] str;

         glGetObjectParameterivARB(fragmentProgram, GL_OBJECT_COMPILE_STATUS_ARB, &success);

         if (!success)
         {
            glGetObjectParameterivARB(fragmentProgram, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);

            GLcharARB * logStr = new GLcharARB[logLength];
            glGetInfoLogARB(fragmentProgram, logLength, NULL, logStr);
            cout << logStr << endl;
            delete [] logStr;

            glDeleteObjectARB(_program);
            _program = 0;
            return;
         } // end if

         checkGLErrorsHere1(_shaderName);

         glAttachObjectARB(_program, fragmentProgram);
         glDeleteObjectARB(fragmentProgram); // To delete the shader if the program is deleted.

         glLinkProgramARB(_program);

         glGetObjectParameterivARB(_program, GL_OBJECT_LINK_STATUS_ARB, &success);

         if (!success)
         {
            glGetObjectParameterivARB(_program, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);

            GLcharARB * logStr = new GLcharARB[logLength];
            glGetInfoLogARB(_program, logLength, NULL, logStr);
            cout << logStr << endl;
            delete [] logStr;

            glDeleteObjectARB(_program);
            _program = 0;
            return;
         } // end if

         checkGLErrorsHere1(_shaderName);

         glUseProgramObjectARB(_program);

         {
            // Build a mapping from texture parameters to texture units.
            _texUnitMap.clear();
            int count, size;
            GLenum type;
            char paramName[1024];

            int texUnit = 0;

            glGetObjectParameterivARB(_program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &count);
            for (int i = 0; i < count; ++i)
            {
               glGetActiveUniformARB(_program, i, 1024, NULL, &size, &type, paramName);

               switch (type)
               {
                  case GL_SAMPLER_1D_ARB:
                  case GL_SAMPLER_2D_ARB:
                  case GL_SAMPLER_3D_ARB:
                  case GL_SAMPLER_CUBE_ARB:
                  case GL_SAMPLER_1D_SHADOW_ARB:
                  case GL_SAMPLER_2D_SHADOW_ARB:
                  case GL_SAMPLER_2D_RECT_ARB:
                  case GL_SAMPLER_2D_RECT_SHADOW_ARB:
                  {
                     _texUnitMap.insert(make_pair(string(paramName), texUnit));
                     int location = glGetUniformLocationARB(_program, paramName);
                     glUniform1iARB(location, texUnit);
                     ++texUnit;
                     break;
                  }
                  default:
                     break;
               } // end switch()
            } // end for (i)
         }
         glUseProgramObjectARB(0);
      } // end if

      checkGLErrorsHere1(_shaderName);
   } // end GLSL_FragmentProgram::compile()

   void
   GLSL_FragmentProgram::compile(std::vector<std::string> const& compilerArgs, char const *entry = 0)
   {
      if (compilerArgs.size() != 0)
         cerr << "GLSL_FragmentProgram::compile(): arguments to the compiler are not supported (and ignored)." << endl;
      if(entry != 0)
          cerr << "GLSL_FragmentProgram::compile(): named entry point is not supported (and ignored)." << endl;
      this->compile();
   }

   void
   GLSL_FragmentProgram::enable()
   {
      glUseProgramObjectARB(_program);
      _inUse = true;
   }

   void 
   GLSL_FragmentProgram::disable()
   {
      _inUse = false;
      glUseProgramObjectARB(0);
   }

   void
   GLSL_FragmentProgram::parameter(char const * param, float x)
   {
      glUseProgramObjectARB(_program);
      glUniform1fARB(glGetUniformLocationARB(_program, param), x);
      if (!_inUse) glUseProgramObjectARB(0);
   }

   void
   GLSL_FragmentProgram::parameter(char const * param, float x, float y)
   {
      glUseProgramObjectARB(_program);
      glUniform2fARB(glGetUniformLocationARB(_program, param), x, y);
      if (!_inUse) glUseProgramObjectARB(0);
   }

   void
   GLSL_FragmentProgram::parameter(char const * param, float x, float y, float z)
   {
      glUseProgramObjectARB(_program);
      glUniform3fARB(glGetUniformLocationARB(_program, param), x, y, z);
      if (!_inUse) glUseProgramObjectARB(0);
   }

   void
   GLSL_FragmentProgram::parameter(char const * param, float x, float y, float z, float w)
   {
      glUseProgramObjectARB(_program);
      glUniform4fARB(glGetUniformLocationARB(_program, param), x, y, z, w);
      if (!_inUse) glUseProgramObjectARB(0);
   }

   void
   GLSL_FragmentProgram::parameter(char const * param, int len, float const * array)
   {
      glUseProgramObjectARB(_program);
      glUniform1fvARB(glGetUniformLocationARB(_program, param), len, array);
      if (!_inUse) glUseProgramObjectARB(0);
   }

   void
   GLSL_FragmentProgram::matrixParameterR(char const * param, int rows, int cols, double const * values)
   {
      float * fvalues = new float[rows*cols];
      std::copy(values, values+rows*cols, fvalues);

      glUseProgramObjectARB(_program);

      if (rows == 2 && cols == 2)
      {
         glUniformMatrix2fvARB(glGetUniformLocationARB(_program, param),
                               1, GL_TRUE, fvalues);
      }
      else if (rows == 3 && cols == 3)
      {
         glUniformMatrix3fvARB(glGetUniformLocationARB(_program, param),
                               1, GL_TRUE, fvalues);
      }
      else if (rows == 4 && cols == 4)
      {
         glUniformMatrix4fvARB(glGetUniformLocationARB(_program, param),
                               1, GL_TRUE, fvalues);
      }
      else
         raiseGLErrorHere2("Matrix parameter should be 2x2, 3x3 or 4x4.", _shaderName.c_str());

      delete [] fvalues;

      if (!_inUse) glUseProgramObjectARB(0);
   }

   void
   GLSL_FragmentProgram::matrixParameterC(char const * param, int rows, int cols, double const * values)
   {
      float * fvalues = new float[rows*cols];
      std::copy(values, values+rows*cols, fvalues);

      glUseProgramObjectARB(_program);

      if (rows == 2 && cols == 2)
      {
         glUniformMatrix2fvARB(glGetUniformLocationARB(_program, param),
                               1, GL_FALSE, fvalues);
      }
      else if (rows == 3 && cols == 3)
      {
         glUniformMatrix3fvARB(glGetUniformLocationARB(_program, param),
                               1, GL_FALSE, fvalues);
      }
      else if (rows == 4 && cols == 4)
      {
         glUniformMatrix4fvARB(glGetUniformLocationARB(_program, param),
                               1, GL_FALSE, fvalues);
      }
      else
         raiseGLErrorHere2("Matrix parameter should be 2x2, 3x3 or 4x4.", _shaderName.c_str());

      delete [] fvalues;

      if (!_inUse) glUseProgramObjectARB(0);
   }

   unsigned
   GLSL_FragmentProgram::getTexUnit(char const * param)
   {
      map<string, int>::const_iterator p = _texUnitMap.find(string(param));
      if (p != _texUnitMap.end())
         return GL_TEXTURE0_ARB + (*p).second;
      else
         raiseGLErrorHere2("Parameter name denotes no texture sampler.", _shaderName.c_str());
      return 0;
   }

} // end namespace V3D_GPU

# endif // defined(V3DLIB_GPGPU_ENABLE_CG)

//----------------------------------------------------------------------

namespace V3D_GPU
{

   void
   setupNormalizedProjection(bool flipY)
   {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      if (!flipY)
         glOrtho(0, 1, 0, 1, -1, 1);
      else
         glOrtho(0, 1, 1, 0, -1, 1);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
   }

   void
   renderNormalizedQuad()
   {
      // It is usually recommended to draw a large (clipped) triangle
      // instread of a single quad, therefore avoiding the diagonal edge.
      glBegin(GL_TRIANGLES);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 0);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 0, 0, 0, 0);
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 0, 0, 0, 0);
      glVertex2f(0, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 2, 0);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 2, 0, 2, 0);
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 2, 0, 2, 0);
      glVertex2f(2, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 2);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 0, 2, 0, 2);
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 0, 2, 0, 2);
      glVertex2f(0, 2);
      glEnd();
   }

   void
   renderNormalizedQuad(GPUTextureSamplingPattern pattern, float ds, float dt)
   {
      switch (pattern)
      {
         case GPU_SAMPLE_NEIGHBORS:
         {
            glBegin(GL_TRIANGLES);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 0,    0);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 0-ds, 0,    0+ds, 0);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 0,    0-dt, 0,    0+dt);
            glVertex2f(0, 0);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 2,    0);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 2-ds, 0,    2+ds, 0);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 2,    0-dt, 2,    0+dt);
            glVertex2f(2, 0);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 0,    2);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 0-ds, 2,    0+ds, 2);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 0,    2-dt, 0,    2+dt);
            glVertex2f(0, 2);
            glEnd();
            break;
         }
         case GPU_SAMPLE_REVERSE_NEIGHBORS:
         {
            glBegin(GL_TRIANGLES);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 0,    0);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 0+ds, 0,    0-ds, 0);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 0,    0+dt, 0,    0-dt);
            glVertex2f(0, 0);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 2,    0);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 2+ds, 0,    2-ds, 0);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 2,    0+dt, 2,    0-dt);
            glVertex2f(2, 0);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 0,    2);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 0+ds, 2,    0-ds, 2);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 0,    2+dt, 0,    2-dt);
            glVertex2f(0, 2);
            glEnd();
            break;
         }
         case GPU_SAMPLE_DIAG_NEIGBORS:
         {
            glBegin(GL_TRIANGLES);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 0,    0);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 0-ds, 0-dt, 0+ds, 0+dt);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 0+ds, 0-dt, 0-ds, 0+dt);
            glVertex2f(0, 0);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 2,    0);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 2-ds, 0-dt, 2+ds, 0+dt);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 2+ds, 0-dt, 2-ds, 0+dt);
            glVertex2f(2, 0);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 0,    2);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 0-ds, 2-dt, 0+ds, 2+dt);
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 0+ds, 2-dt, 0-ds, 2+dt);
            glVertex2f(0, 2);
            glEnd();
            break;
         }
         case GPU_SAMPLE_2X2_BLOCK:
         {
            glBegin(GL_TRIANGLES);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 0);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 0, 0,    0+ds, 0   );
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 0, 0+dt, 0+ds, 0+dt);
            glVertex2f(0, 0);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 2, 0);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 2, 0,    2+ds, 0   );
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 2, 0+dt, 2+ds, 0+dt);
            glVertex2f(2, 0);
            glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 2);
            glMultiTexCoord4f(GL_TEXTURE1_ARB, 0, 2,    0+ds, 2   );
            glMultiTexCoord4f(GL_TEXTURE2_ARB, 0, 2+dt, 0+ds, 2+dt);
            glVertex2f(0, 2);
            glEnd();
            break;
         }
         default:
            raiseGLErrorHere1("Unknown sampling pattern.");
      } // end switch (pattern)
   } // end renderQuadWithTexSampling()

} // end namespace V3D_GPU

# if defined(V3DLIB_GPGPU_ENABLE_CG)

namespace
{
   V3D_GPU::Cg_FragmentProgram * trivialTexture2DShader = 0;
}

namespace V3D_GPU
{

   void
   enableTrivialTexture2DShader()
   {
      if (trivialTexture2DShader == 0)
      {
         trivialTexture2DShader = new Cg_FragmentProgram("trivialTexture2DShader");
         char const * source =
            "void main(uniform sampler2D texture, \n"
            "                  float2 st : TEXCOORD0, \n"
            "              out float4 color : COLOR) \n"
            "{ \n"
            "   color = tex2D(texture, st); \n"
            "} \n";
         trivialTexture2DShader->setProgram(source);
         trivialTexture2DShader->compile();
         checkGLErrorsHere0();
      }
      trivialTexture2DShader->enable();
   }

   void
   disableTrivialTexture2DShader()
   {
      if (trivialTexture2DShader) trivialTexture2DShader->disable();
   }

} // end namespace V3D_GPU

# endif

#endif // defined(V3DLIB_ENABLE_GPGPU)
