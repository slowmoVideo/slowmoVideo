// -*- C++ -*-
#ifndef V3D_GPUBASE_H
#define V3D_GPUBASE_H

# if defined(V3DLIB_ENABLE_GPGPU)

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include <GL/glew.h>

#define checkGLErrorsHere0() { V3D_GPU::checkGLErrors(__FILE__, __LINE__); }
#define checkGLErrorsHere1(NAME) { V3D_GPU::checkGLErrors(__FILE__, __LINE__, NAME); }
#define raiseGLErrorHere1(MSG) { V3D_GPU::raiseGLError(__FILE__, __LINE__, MSG); }
#define raiseGLErrorHere2(MSG, NAME) { V3D_GPU::raiseGLError(__FILE__, __LINE__, MSG, NAME); }

# if defined(V3DLIB_GPGPU_ENABLE_CG)

struct _CGcontext;
typedef _CGcontext * CGcontext;

struct _CGprogram;
typedef _CGprogram * CGprogram;

# endif

namespace V3D_GPU
{
   typedef unsigned char uchar;

   struct TextureSpecification
   {
         TextureSpecification()
            : nChannels(3), nBitsPerChannel(8), isFloatTexture(false), isDepthTexture(false),
              isRTT(true), enableTextureRG(false)
         { }

         TextureSpecification(char const * specString);

         unsigned int getGLInternalFormat() const;

         uchar nChannels, nBitsPerChannel;
         bool isFloatTexture, isDepthTexture;
         bool isRTT, enableTextureRG;
   }; // end struct TextureSpecification

   struct ImageTexture2D
   {
         ImageTexture2D(char const * texName = "<unnamed 2D texture>")
            : _texName(texName), _textureID(0), _textureTarget(0), _width(0), _height(0)
         { }

         virtual ~ImageTexture2D() { }

         bool allocateID();
         void deallocateID();

         void reserve(int width, int height, TextureSpecification const& texSpec);

         void overwriteWith(uchar const * pixels, int nChannels);
         void overwriteWith(uchar const * redPixels, uchar const * greenPixels, uchar const * bluePixels);
         void overwriteWith(uchar const * redPixels, uchar const * greenPixels,
                            uchar const * bluePixels, uchar const * alphaPixels);

         void overwriteWith(float const * pixels, int nChannels);
         void overwriteWith(float const * redPixels, float const * greenPixels, float const * bluePixels);
         void overwriteWith(float const * redPixels, float const * greenPixels,
                            float const * bluePixels, float const * alphaPixels);

         void clear();

         void bind();
         void bind(unsigned texUnit);

         void enable();
         void enable(unsigned texUnit);
         void disable();
         void disable(unsigned texUnit);

         unsigned width()         const { return _width; }
         unsigned height()        const { return _height; }
         unsigned textureID()     const { return _textureID; }
         unsigned textureTarget() const { return _textureTarget; }

      protected:
         std::string _texName;

         unsigned _textureID;
         unsigned _textureTarget;
         unsigned _width, _height;
   }; // end struct ImageTexture2D

   struct FrameBufferObject
   {
         FrameBufferObject(char const * fboName = "<unnamed frame buffer object>")
            : _fboName(fboName), _attachedDepthTexture(0)
         {
            std::fill(_attachedColorTextures, _attachedColorTextures+16, (ImageTexture2D *)0);
         }

         virtual ~FrameBufferObject() { }

         bool allocate();
         void deallocate();

         void makeCurrent();
         void activate(bool setViewport = true);

         bool isCurrent();

         void attachTexture2D(ImageTexture2D& texture,
                              GLenum attachment = GL_COLOR_ATTACHMENT0_EXT,
                              int mipLevel = 0);
         void attachTextures2D(int numTextures, ImageTexture2D * textures,
                               GLenum * attachment = 0, int * mipLevel = 0);

         void detach(GLenum attachment);
         void detachAll();

# ifndef NDEBUG_GL
         bool checkValidity(std::ostream& os = std::cerr);
# else
         bool checkValidity(std::ostream& os = std::cerr) { return true; }
# endif

         unsigned width() const
         {
            for (int i = 0; i < 16; ++i)
               if (_attachedColorTextures[i])
                  return _attachedColorTextures[i]->width();
            return 0;
         }

         unsigned height() const
         {
            for (int i = 0; i < 16; ++i)
               if (_attachedColorTextures[i])
                  return _attachedColorTextures[i]->height();
            return 0;
         }

         unsigned frameBufferID() const { return _fboID; }

         ImageTexture2D& getColorTexture(int i) const { return *_attachedColorTextures[i]; }

         static int getMaxColorAttachments();

         static void disableFBORendering();

      protected:
         bool checkBinding(char const * what);

         std::string      _fboName;
         GLuint           _fboID;
         ImageTexture2D * _attachedColorTextures[16];
         ImageTexture2D * _attachedDepthTexture;
   }; // end struct FrameBufferObject

   struct RTT_Buffer
   {
         RTT_Buffer(char const * texSpec = "rgb=8",
                    char const * rttName = "<unnamed RTT buffer object>")
            : _texSpec(texSpec), _tex(rttName), _fbo(rttName)
         { }

         virtual ~RTT_Buffer() { }

         bool allocate(int const w, int const h)
         {
            _tex.allocateID();
            _tex.reserve(w, h, TextureSpecification(_texSpec.c_str()));
            _fbo.allocate();
            _fbo.makeCurrent();
            _fbo.attachTexture2D(_tex);
            _fbo.checkValidity();
            return true;
         }

         bool allocate(char const * texSpec, int const w, int const h)
         {
            _texSpec = texSpec;
            return allocate(w,h);
         }

         bool reallocate(int w, int h)
         {
            _tex.reserve(w, h, TextureSpecification(_texSpec.c_str()));
            return true;
         }

         void deallocate()
         {
            _fbo.deallocate();
            _tex.deallocateID();
         }

         unsigned width()         const { return _tex.width(); }
         unsigned height()        const { return _tex.height(); }
         unsigned textureID()     const { return _tex.textureID(); }
         unsigned textureTarget() const { return _tex.textureTarget(); }

         void bindTexture()                    { _tex.bind(); }
         void bindTexture(unsigned texUnit)    { _tex.bind(texUnit); }
         void enableTexture()                  { _tex.enable(); }
         void enableTexture(unsigned texUnit)  { _tex.enable(texUnit); }
         void disableTexture()                 { _tex.disable(); }
         void disableTexture(unsigned texUnit) { _tex.disable(texUnit); }

         void makeCurrent() { _fbo.makeCurrent(); }
         void activate(bool setViewport = true) { _fbo.activate(setViewport); }
         bool isCurrent() { return _fbo.isCurrent(); }

         ImageTexture2D&    getTexture() { return _tex; }
         FrameBufferObject& getFBO()     { return _fbo; }

      protected:
         std::string _texSpec;

         ImageTexture2D    _tex;
         FrameBufferObject _fbo;
   }; // end struct RTT_Buffer

//----------------------------------------------------------------------

   struct ProgramBase
   {
         ProgramBase(char const * shaderName) 
            : _shaderName(shaderName)
         { }

         virtual ~ProgramBase() { }

         virtual void setProgram(char const * source) = 0;
         virtual void setProgram(std::string const& source) { this->setProgram(source.c_str()); }

         virtual void setProgramFromFile(char const * fileName);

         virtual void compile(char const * * compilerArgs = 0, char const *entry = 0) = 0;
         virtual void compile(std::vector<std::string> const& compilerArgs, char const *entry = 0) = 0;
         virtual void enable() = 0;
         virtual void disable() = 0;

         virtual void parameter(char const * param, float x) = 0;
         virtual void parameter(char const * param, float x, float y) = 0;
         virtual void parameter(char const * param, float x, float y, float z) = 0;
         virtual void parameter(char const * param, float x, float y, float z, float w) = 0;
         virtual void parameter(char const * param, int len, float const * array) = 0;
         virtual void matrixParameterR(char const * param, int rows, int cols, double const * values) = 0;
         virtual void matrixParameterC(char const * param, int rows, int cols, double const * values) = 0;

         virtual unsigned getTexUnit(char const * param) = 0;

         std::string const& shaderName() const { return _shaderName; }

      protected:
         std::string _shaderName;
   };

#  if defined(V3DLIB_GPGPU_ENABLE_CG)
   struct Cg_ProgramBase
   {
         Cg_ProgramBase() 
            : _source(), _program(0)
         { }

         virtual ~Cg_ProgramBase();

         char const * getCompiledString();

         static void initializeCg();

      protected:
         static void handleCgError() ;

         std::string _source;
         CGprogram   _program;

         static CGcontext _context;
   };

   struct Cg_FragmentProgram : public ProgramBase, public Cg_ProgramBase
   {
         Cg_FragmentProgram(char const * shaderName)
            : ProgramBase(shaderName), Cg_ProgramBase()
         { }

         virtual ~Cg_FragmentProgram() { }

         virtual void setProgram(char const * source);

         virtual void compile(char const * * compilerArgs = 0, char const *entry = 0);
         virtual void compile(std::vector<std::string> const& compilerArgs, char const *entry = 0);
         virtual void enable();
         virtual void disable();

         virtual void parameter(char const * param, float x);
         virtual void parameter(char const * param, float x, float y);
         virtual void parameter(char const * param, float x, float y, float z);
         virtual void parameter(char const * param, float x, float y, float z, float w);
         virtual void parameter(char const * param, int len, float const * array);
         virtual void matrixParameterR(char const * param, int rows, int cols, double const * values);
         virtual void matrixParameterC(char const * param, int rows, int cols, double const * values);

         virtual unsigned getTexUnit(char const * param);
   };
#  endif // defined(V3DLIB_GPGPU_ENABLE_CG)

#  if defined(V3DLIB_GPGPU_ENABLE_GLSL)
   struct GLSL_FragmentProgram : public ProgramBase
   {
         GLSL_FragmentProgram(char const * shaderName)
            : ProgramBase(shaderName), _source(), _program(0), _inUse(false), _texUnitMap()
         { }

         virtual ~GLSL_FragmentProgram() { }

         virtual void setProgram(char const * source);

         virtual void compile(char const * * compilerArgs = 0, char const *entry = 0);
         virtual void compile(std::vector<std::string> const& compilerArgs, char const *entry = 0);
         virtual void enable();
         virtual void disable();

         virtual void parameter(char const * param, float x);
         virtual void parameter(char const * param, float x, float y);
         virtual void parameter(char const * param, float x, float y, float z);
         virtual void parameter(char const * param, float x, float y, float z, float w);
         virtual void parameter(char const * param, int len, float const * array);
         virtual void matrixParameterR(char const * param, int rows, int cols, double const * values);
         virtual void matrixParameterC(char const * param, int rows, int cols, double const * values);

         virtual unsigned getTexUnit(char const * param);

      protected:
         std::string _source;
         GLhandleARB _program;
         bool        _inUse;

         std::map<std::string, int> _texUnitMap;
   };
#  endif // defined(V3DLIB_GPGPU_ENABLE_GLSL)

//----------------------------------------------------------------------

#  ifndef NDEBUG_GL
   void checkGLErrors(char const * location, std::ostream& os = std::cerr);
   void checkGLErrors(char const * file, int line, std::ostream& os = std::cerr);
   void checkGLErrors(char const * file, int line, std::string const& name, std::ostream& os = std::cerr);
   bool checkFrameBufferStatus(char const * file, int line, std::string const& name, std::ostream& os = std::cerr);
#  else
   inline void checkGLErrors(char const * location, std::ostream& os = std::cerr) { }
   inline void checkGLErrors(char const * file, int line, std::ostream& os = std::cerr) { }
   inline void checkGLErrors(char const * file, int line, std::string const& name, std::ostream& os = std::cerr) { }
   inline bool checkFrameBufferStatus(char const * file, int line, std::string const& name, std::ostream& os = std::cerr)
   {
      return true;
   }
#  endif

   void raiseGLError(char const * file, int line, char const * msg, std::ostream& os = std::cerr);
   void raiseGLError(char const * file, int line, char const * msg, std::string const& name, std::ostream& os = std::cerr);


//----------------------------------------------------------------------

   enum GPUTextureSamplingPattern
   {
      GPU_SAMPLE_NONE = 0,
      GPU_SAMPLE_NEIGHBORS = 1, //!< Sample the 4 direct neighbors (E, W, N, S)
      GPU_SAMPLE_REVERSE_NEIGHBORS = 2, //!< Sample the 4 direct neighbors (W, E, S, N)
      GPU_SAMPLE_DIAG_NEIGBORS = 3, //!< Sample the diagonal neighbors (NE, SW, NW, SE)
      GPU_SAMPLE_2X2_BLOCK = 4, //!< Sample the 2x2 block (here, E, S, SE)
   };

   void setupNormalizedProjection(bool flipY = false);
   void renderNormalizedQuad();
   void renderNormalizedQuad(GPUTextureSamplingPattern pattern, float ds, float dt);

#  if defined(V3DLIB_GPGPU_ENABLE_CG)
   void enableTrivialTexture2DShader();
   void disableTrivialTexture2DShader();
#  endif

} // end namespace V3D_GPU

# endif // defined(V3DLIB_ENABLE_GPGPU)

#endif // defined(V3D_GPUBASE_H)
