#if defined(V3DLIB_GPGPU_ENABLE_CG)

#include "v3d_gpuklt.h"

#include <GL/glew.h>

#include <algorithm>
#include <iostream>
#include <cstdio>

//#define USE_VBO_RENDERING 1
#define SHUFFLE_FEATURES 1
#define USE_PARTIAL_SORT 1

using namespace std;
using namespace V3D_GPU;

namespace
{

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

   inline void
   render2x2Tap(float shiftS, float shiftT, float dS, float dT)
   {
      glBegin(GL_TRIANGLES);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 0);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 0+shiftS, 0+shiftT,    0+shiftS+dS, 0+shiftT   );
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 0+shiftS, 0+shiftT+dT, 0+shiftS+dS, 0+shiftT+dT);
      glVertex2f(0, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 2, 0);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 2+shiftS, 0+shiftT,    2+shiftS+dS, 0+shiftT   );
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 2+shiftS, 0+shiftT+dT, 2+shiftS+dS, 0+shiftT+dT);
      glVertex2f(2, 0);
      glMultiTexCoord2f(GL_TEXTURE0_ARB, 0, 2);
      glMultiTexCoord4f(GL_TEXTURE1_ARB, 0+shiftS, 2+shiftT,    0+shiftS+dS, 2+shiftT   );
      glMultiTexCoord4f(GL_TEXTURE2_ARB, 0+shiftS, 2+shiftT+dT, 0+shiftS+dS, 2+shiftT+dT);
      glVertex2f(0, 2);
      glEnd();
   } // end render2x2Tap()

} // end namespace <>


void
KLT_Tracker::allocate(int width, int height, int featureWidth, int featureHeight)
{
   _width = width;
   _height = height;
   _featureWidth = featureWidth;
   _featureHeight = featureHeight;

   _featuresBufferA.allocate(featureWidth, featureHeight);
   _featuresBufferB.allocate(featureWidth, featureHeight);
   checkGLErrorsHere0();

   _featuresBuffer0 = &_featuresBufferA;
   _featuresBuffer1 = &_featuresBufferB;
}

void
KLT_Tracker::deallocate()
{
   checkGLErrorsHere0();
   _featuresBufferA.deallocate();
   _featuresBufferB.deallocate();
   checkGLErrorsHere0();
}

void
KLT_Tracker::provideFeatures(float const * features)
{
//    if (_featuresBuffer1->isCurrent())
//       FrameBufferObject::disableFBORendering();

   _featuresBuffer1->bindTexture();
   glTexSubImage2D(_featuresBuffer1->textureTarget(),
                   0, 0, 0, _featureWidth, _featureHeight,
                   GL_RGB, GL_FLOAT, features);
   glBindTexture(_featuresBuffer1->textureTarget(), 0);
}

void
KLT_Tracker::readFeatures(float * features)
{
   _featuresBuffer1->activate();
   glReadPixels(0, 0, _featureWidth, _featureHeight, GL_RGB, GL_FLOAT, features);
}

void
KLT_Tracker::trackFeatures(unsigned int pyrTex0, unsigned int pyrTex1)
{
   if (_trackingShader == 0)
   {
      _trackingShader = new Cg_FragmentProgram("KLT_Tracker::_trackingShader");
      _trackingShader->setProgramFromFile("klt_tracker.cg");

      vector<string> args;
      //args.push_back("-unroll"); args.push_back("all");
      //char const * args[] = { "-profile", "gp4fp", 0 };
      char str[512];
      sprintf(str, "-DNITERATIONS=%i", _nIterations);
      args.push_back(str);
      sprintf(str, "-DN_LEVELS=%i", _nLevels);
      args.push_back(str);
      sprintf(str, "-DLEVEL_SKIP=%i", _levelSkip);
      args.push_back(str);
      sprintf(str, "-DHALF_WIDTH=%i", _windowWidth/2);
      args.push_back(str);
      _trackingShader->compile(args);
      //cout << shader->getCompiledString() << endl;
      checkGLErrorsHere0();
   } // end if (shader == 0)

   setupNormalizedProjection();
   _featuresBuffer1->activate();

   float const ds = 1.0f / _width;
   float const dt = 1.0f / _height;

   _featuresBuffer0->enableTexture(GL_TEXTURE0_ARB);

   glActiveTexture(GL_TEXTURE1_ARB);
   glBindTexture(GL_TEXTURE_2D, pyrTex0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
   glEnable(GL_TEXTURE_2D);

   glActiveTexture(GL_TEXTURE2_ARB);
   glBindTexture(GL_TEXTURE_2D, pyrTex1);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
   glEnable(GL_TEXTURE_2D);

   _trackingShader->parameter("ds", ds, dt);
   _trackingShader->parameter("wh", _width, _height);
   _trackingShader->parameter("sqrConvergenceThreshold", _convergenceThreshold*_convergenceThreshold);
   _trackingShader->parameter("SSD_Threshold", _SSD_Threshold);
   _trackingShader->parameter("validRegion", _margin/_width, _margin/_height,
                              1.0f - _margin/_width, 1.0f - _margin/_height);
   _trackingShader->enable();
   renderNormalizedQuad();
   _trackingShader->disable();

   _featuresBuffer0->disableTexture(GL_TEXTURE0_ARB);
   glActiveTexture(GL_TEXTURE1_ARB);
   glDisable(GL_TEXTURE_2D);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glActiveTexture(GL_TEXTURE2_ARB);
   glDisable(GL_TEXTURE_2D);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
} // end KLT_Tracker::trackFeatures()

//----------------------------------------------------------------------

void
KLT_TrackerWithGain::allocate(int width, int height, int featureWidth, int featureHeight)
{
   _width = width;
   _height = height;
   _featureWidth = featureWidth;
   _featureHeight = featureHeight;

   _featuresBufferA.allocate(featureWidth, featureHeight);
   _featuresBufferB.allocate(featureWidth, featureHeight);
   _featuresBufferC.allocate(featureWidth, featureHeight);
   checkGLErrorsHere0();

   _featuresBuffer0 = &_featuresBufferA;
   _featuresBuffer1 = &_featuresBufferB;
}

void
KLT_TrackerWithGain::deallocate()
{
   checkGLErrorsHere0();
   _featuresBufferA.deallocate();
   _featuresBufferB.deallocate();
   _featuresBufferC.deallocate();
   checkGLErrorsHere0();
}

void
KLT_TrackerWithGain::provideFeaturesAndGain(float const * features)
{
//    if (_featuresBuffer2->isCurrent())
//       FrameBufferObject::disableFBORendering();

   _featuresBuffer2->bindTexture();
   glTexSubImage2D(_featuresBuffer2->textureTarget(),
                   0, 0, 0, _featureWidth, _featureHeight,
                   GL_RGB, GL_FLOAT, features);
   _featuresBuffer1->bindTexture();
   glTexSubImage2D(_featuresBuffer1->textureTarget(),
                   0, 0, 0, _featureWidth, _featureHeight,
                   GL_RGB, GL_FLOAT, features);
   glBindTexture(_featuresBuffer1->textureTarget(), 0);
}

void
KLT_TrackerWithGain::readFeaturesAndGain(float * features)
{
   _featuresBuffer2->activate();
   glReadPixels(0, 0, _featureWidth, _featureHeight, GL_RGB, GL_FLOAT, features);
   //glReadPixels(0, 0, _featureWidth, _featureHeight, GL_BGR, GL_FLOAT, features);
}

void
KLT_TrackerWithGain::trackFeaturesAndGain(unsigned int pyrTex0, unsigned int pyrTex1)
{
   if (_trackingShader == 0)
   {
      _trackingShader = new Cg_FragmentProgram("KLT_TrackerWithGain::_trackingShader");
      _trackingShader->setProgramFromFile("klt_tracker_with_gain.cg");

      vector<string> args;
      char str[512];
      sprintf(str, "-DHALF_WIDTH=%i", _windowWidth/2);
      args.push_back(str);

      _trackingShader->compile(args);

      //cout << shader->getCompiledString() << endl;
      checkGLErrorsHere0();
   } // end if (shader == 0)

   setupNormalizedProjection();

   _featuresBuffer0->activate();
   glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
   glClearColor(0, 0, 1, 0);
   glClear(GL_COLOR_BUFFER_BIT);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

   _featuresBuffer2->enableTexture(GL_TEXTURE3_ARB);

   glActiveTexture(GL_TEXTURE1_ARB);
   glBindTexture(GL_TEXTURE_2D, pyrTex0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glEnable(GL_TEXTURE_2D);

   glActiveTexture(GL_TEXTURE2_ARB);
   glBindTexture(GL_TEXTURE_2D, pyrTex1);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glEnable(GL_TEXTURE_2D);

   float delta = 200.0f;
   float const tau = 1.0f;

   _trackingShader->parameter("wh", _width, _height);
   _trackingShader->parameter("sqrConvergenceThreshold", 1000000.0f);
   _trackingShader->parameter("SSD_Threshold", 1000000.0f);
   _trackingShader->parameter("validRegion", -1.0f, -1.0f, 2.0f, 2.0f);
   _trackingShader->parameter("lambda", 1.0f);
   _trackingShader->parameter("ds0", 1.0f/_featureWidth, 1.0f/_featureHeight);
   _trackingShader->enable();

   for (int level = _nLevels-1; level >= 0; level -= _levelSkip)
   {
      int const w = _width >> level;
      int const h = _height >> level;
      float const ds = 1.0f / w;
      float const dt = 1.0f / h;

      _trackingShader->parameter("ds", ds, dt);

      glActiveTexture(GL_TEXTURE1_ARB);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);

      glActiveTexture(GL_TEXTURE2_ARB);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);

      for (int iter = 1; iter <= _nIterations; ++iter)
      {
         _trackingShader->parameter("delta", delta);
         delta *= tau;
         if (iter == 1)
         {
            _trackingShader->parameter("sqrConvergenceThreshold", 1000000.0f);
            _trackingShader->parameter("SSD_Threshold", 1000000.0f);
            _trackingShader->parameter("validRegion", -1.0f, -1.0f, 2.0f, 2.0f);
         }
         else if (iter == _nIterations)
         {
            _trackingShader->parameter("sqrConvergenceThreshold", _convergenceThreshold*_convergenceThreshold);
            _trackingShader->parameter("SSD_Threshold", _SSD_Threshold);
            _trackingShader->parameter("validRegion", _margin/_width, _margin/_height, 1.0f - _margin/_width, 1.0f - _margin/_height);
         }

         _featuresBuffer1->activate();
         _featuresBuffer0->enableTexture(GL_TEXTURE0_ARB);
         renderNormalizedQuad();
         _featuresBuffer0->disableTexture(GL_TEXTURE0_ARB);
         std::swap(_featuresBuffer0, _featuresBuffer1);
      } // end for (iter)
   } // end for (level)

   _trackingShader->disable();

   glActiveTexture(GL_TEXTURE1_ARB);
   glDisable(GL_TEXTURE_2D);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glActiveTexture(GL_TEXTURE2_ARB);
   glDisable(GL_TEXTURE_2D);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   _featuresBuffer2->disableTexture(GL_TEXTURE3_ARB);

   std::swap(_featuresBuffer0, _featuresBuffer2);
} // end KLT_TrackerWithGain::trackFeatures()

//----------------------------------------------------------------------

void
KLT_Detector::allocate(int width, int height, int pointListWidth, int pointListHeight)
{
   if (_cornernessShader1 != 0)
   {
      cerr << "KLT_Detector::allocate(): Did you call allocate() twice?" << endl;
      return;
   }

   checkGLErrorsHere0();

   _width = width;
   _height = height;
   _pointListWidth = pointListWidth;
   _pointListHeight = pointListHeight;

   //_featuresBuffer.allocate(featureWidth, featureHeight);
   _convRowsBuffer.allocate(width, height);
   _cornernessBuffer.allocate(width, height);
   _nonmaxRowsBuffer.allocate(width, height);
   _pointListBuffer.allocate(pointListWidth, pointListHeight);
   checkGLErrorsHere0();

   // The histogram pyramid is a POT, find the next POT value >= max(width, height).
   _histpyrWidth = 1;
   _nHistpyrLevels = 0;
   while (_histpyrWidth < std::max(width, height))
   {
      _histpyrWidth *= 2;
      ++_nHistpyrLevels;
   }
   _histpyrWidth = (1 << (_nHistpyrLevels-1));

//    cout << "_histpyrWidth = " << _histpyrWidth << endl;
//    cout << "_nHistpyrLevels = " << _nHistpyrLevels << endl;

   glGenTextures(1, &_histpyrTexID);
   glGenFramebuffersEXT(_nHistpyrLevels, _histpyrFbIDs);

   glBindTexture(GL_TEXTURE_2D, _histpyrTexID);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   // This is the simplest way to create the full mipmap pyramid - temporary enable auto mipmap generation.
   glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
   std::vector<unsigned char> pixels(_histpyrWidth * _histpyrWidth * 3, 0);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, _histpyrWidth, _histpyrWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
   glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); 

   for (int k = 0; k < _nHistpyrLevels; ++k)
   {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _histpyrFbIDs[k]);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _histpyrTexID, k);
      bool status = checkFrameBufferStatus(__FILE__, __LINE__, "histo pyramid buffer");
   }

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

   // Those are per instance, not static, in order to have individual windows sizes etc.
   {
      _cornernessShader1 = new Cg_FragmentProgram("KLT_Detector::_cornernessShader1");
      _cornernessShader1->setProgramFromFile("klt_detector_pass1.cg");
      _cornernessShader1->compile();
      checkGLErrorsHere0();
   }

   {
      _cornernessShader2 = new Cg_FragmentProgram("KLT_Detector::_cornernessShader2");
      _cornernessShader2->setProgramFromFile("klt_detector_pass2.cg");
      _cornernessShader2->compile();
      checkGLErrorsHere0();
   }

   {
      char buf[200];
      sprintf(buf, "-DMIN_DIST=%i", _minDist);

      _nonmaxShader = new Cg_FragmentProgram("KLT_Detector::_nonmaxShader");
      _nonmaxShader->setProgramFromFile("klt_detector_nonmax.cg");
      char const * args[] = { "-unroll", "all", buf, 0 };
      _nonmaxShader->compile(args);
      checkGLErrorsHere0();
   }

   {
      char buf[200];
      sprintf(buf, "-DPYR_LEVELS=%i", _nHistpyrLevels);

      _traverseShader = new Cg_FragmentProgram("KLT_Detector::_traverseShader");
      _traverseShader->setProgramFromFile("klt_detector_traverse_histpyr.cg");
      char const * args[] = { "-unroll", "all", buf, 0 };
      _traverseShader->compile(args);
      checkGLErrorsHere0();
   }

#if defined(USE_VBO_RENDERING)
   glGenBuffersARB(1, &_vbo);
   glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vbo);
   glBufferData(GL_ARRAY_BUFFER_ARB, pointListWidth*pointListHeight*3*sizeof(float), NULL, GL_DYNAMIC_DRAW_ARB);
   glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
#endif
   checkGLErrorsHere0();
} // end KLT_Detector::allocate()

void
KLT_Detector::deallocate()
{
   checkGLErrorsHere0();
   _convRowsBuffer.deallocate();
   _cornernessBuffer.deallocate();
   _nonmaxRowsBuffer.deallocate();
   _pointListBuffer.deallocate();
   glDeleteFramebuffersEXT(_nHistpyrLevels, _histpyrFbIDs);
   glDeleteTextures(1, &_histpyrTexID);
#if defined(USE_VBO_RENDERING)
   glDeleteBuffersARB(1, &_vbo);
#endif
   checkGLErrorsHere0();
}

void
KLT_Detector::detectCorners(float const minCornerness, unsigned int texID, int& nFeatures,
                            int nPresentFeatures, float * presentFeaturesBuffer)
{
   static Cg_FragmentProgram * discrimiatorShader = 0;
   static Cg_FragmentProgram * buildHistpyrShader = 0;
   static Cg_FragmentProgram * presentFeaturesShader = 0;

   if (discrimiatorShader == 0)
   {
      discrimiatorShader = new Cg_FragmentProgram("KLT_Detector::detectCorners()::discrimiatorShader");
      discrimiatorShader->setProgramFromFile("klt_detector_discriminator.cg");
      discrimiatorShader->compile();
      checkGLErrorsHere0();
   } // end if (discrimiatorShader == 0)

   if (buildHistpyrShader == 0)
   {
      buildHistpyrShader = new Cg_FragmentProgram("KLT_Detector::detectCorners()::buildHistpyrShader");
      buildHistpyrShader->setProgramFromFile("klt_detector_build_histpyr.cg");
      buildHistpyrShader->compile();
      checkGLErrorsHere0();
   } // end if (buildHistpyrShader == 0)

   if (presentFeaturesShader == 0)
   {
      presentFeaturesShader = new Cg_FragmentProgram("KLT_Detector::detectCorners()::presentFeaturesShader");
      char const * source =
         "void main(out float4 color : COLOR) \n"
         "{ \n"
         "   color = unpack_4ubyte(-1e30); \n"
         "} \n";
      presentFeaturesShader->setProgram(source);
      presentFeaturesShader->compile();
      checkGLErrorsHere0();
   } // end if (presentFeaturesShader == 0)

   checkGLErrorsHere0();

   setupNormalizedProjection();

   // Compute the cornerness
   _convRowsBuffer.activate();
   glActiveTexture(GL_TEXTURE0_ARB);
   glBindTexture(GL_TEXTURE_2D, texID);
   glEnable(GL_TEXTURE_2D);

   _cornernessShader1->enable();
   renderQuad8Tap(0.0f, 1.0f/_height);
   _cornernessShader1->disable();

   _cornernessBuffer.activate();
   _convRowsBuffer.bindTexture();
   _cornernessShader2->parameter("minCornerness", minCornerness);
   _cornernessShader2->parameter("validRegion", _margin/_width, _margin/_height,
                                 1.0f - _margin/_width, 1.0f - _margin/_height);
   _cornernessShader2->enable();
   renderQuad8Tap(1.0f/_width, 0.0f);
   _cornernessShader2->disable();

   if (nPresentFeatures > 0)
   {
      // Suppress corner detection in the vicinity of still valid tracks.
      glPointSize(1);
      presentFeaturesShader->enable();
#if !defined(USE_VBO_RENDERING)
      // Traditional vertex array version, no performance difference.
      glVertexPointer(2, GL_FLOAT, 3*sizeof(float), presentFeaturesBuffer);
      glEnableClientState(GL_VERTEX_ARRAY);
      glDrawArrays(GL_POINTS, 0, nPresentFeatures);
      glDisableClientState(GL_VERTEX_ARRAY);
#else
      // Vertex buffer object version
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vbo);
      float * p = (float *)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);

      memcpy(p, presentFeaturesBuffer, 3*nPresentFeatures*sizeof(float));

      glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
      glVertexPointer(2, GL_FLOAT, 3*sizeof(float), 0);
      glEnableClientState(GL_VERTEX_ARRAY);
      glDrawArrays(GL_POINTS, 0, nPresentFeatures);
      glDisableClientState(GL_VERTEX_ARRAY);
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
#endif
      presentFeaturesShader->disable();
   }

   // Apply non-max suppression
   _nonmaxRowsBuffer.activate();
   _cornernessBuffer.bindTexture();
   _nonmaxShader->parameter("ds", 1.0f/_width, 0.0f);
   _nonmaxShader->enable();
   renderNormalizedQuad();
   _cornernessBuffer.activate();
   _nonmaxRowsBuffer.bindTexture();
   _nonmaxShader->parameter("ds", 0.0f, 1.0f/_height);
   renderNormalizedQuad();
   _nonmaxShader->disable();

   // Note: cornerness > 0 denotes an active features, cornerss < 0 a suppressed one.

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _histpyrFbIDs[0]);
   glClearColor(0, 0, 0, 0);
   glClear(GL_COLOR_BUFFER_BIT);
   glViewport(0, 0, _width/2, _height/2);
   glBindTexture(GL_TEXTURE_2D, _cornernessBuffer.textureID());
   discrimiatorShader->parameter("wh", _width, _height);
   discrimiatorShader->enable();
   render2x2Tap(-0.5f/_width, -0.5f/_height, 1.0f/_width, 1.0f/_height);
   discrimiatorShader->disable();

   glBindTexture(GL_TEXTURE_2D, _histpyrTexID);
   buildHistpyrShader->enable();

   for (int k = 1; k < _nHistpyrLevels; ++k)
   {
      int const W = (_histpyrWidth >> k);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _histpyrFbIDs[k]);
      glViewport(0, 0, W, W);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, k-1);
      render2x2Tap(-0.25f/W, -0.25f/W, 0.5f/W, 0.5f/W);
   } // end for (k)

   buildHistpyrShader->disable();
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
   glDisable(GL_TEXTURE_2D);
   checkGLErrorsHere0();

   float vals[4];
   glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, vals);

   nFeatures = int(vals[0] + vals[1] + vals[2] + vals[3]);
} // end KLT_Detector::detectCorners()

void
KLT_Detector::extractCorners(int const nFeatures, float * dest)
{
   _pointListBuffer.activate();
   glClearColor(-1, -1, -1, 0);
   glClear(GL_COLOR_BUFFER_BIT);

   int const nRequiredRows = (nFeatures + _pointListWidth - 1) / _pointListWidth;
   //cout << "nRequiredRows = " << nRequiredRows << endl;
   glViewport(0, 0, _pointListWidth, nRequiredRows);

   glActiveTexture(GL_TEXTURE0_ARB);
   glBindTexture(GL_TEXTURE_2D, _histpyrTexID);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
   glEnable(GL_TEXTURE_2D);

   _cornernessBuffer.enableTexture(GL_TEXTURE1_ARB);

   //cout << "_pointListWidth = " << _pointListWidth << endl;
   _traverseShader->parameter("nFeatures", nFeatures);
   _traverseShader->parameter("pointListWidth", _pointListWidth);
   _traverseShader->parameter("pyrW", _histpyrWidth);
   _traverseShader->parameter("srcWH", _width, _height);
   _traverseShader->enable();

   glBegin(GL_TRIANGLES);
   glMultiTexCoord2f(GL_TEXTURE0_ARB, -0.5f, -0.5f);
   glVertex2f(0, 0);
   glMultiTexCoord2f(GL_TEXTURE0_ARB, 2*_pointListWidth-0.5f, -0.5f);
   glVertex2f(2, 0);
   glMultiTexCoord2f(GL_TEXTURE0_ARB, -0.5f, 2*nRequiredRows-0.5f);
   glVertex2f(0, 2);
   glEnd();

   _traverseShader->disable();
   _cornernessBuffer.disableTexture(GL_TEXTURE1_ARB);

   glActiveTexture(GL_TEXTURE0_ARB);
   glDisable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, _histpyrTexID);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   glReadPixels(0, 0, _pointListWidth, nRequiredRows, GL_RGB, GL_FLOAT, dest);
} // end KLT_Detector::extractCorners()

//----------------------------------------------------------------------

void
KLT_SequenceTracker::allocate(int width, int height, int nLevels,
                              int featuresWidth, int featuresHeight,
                              int pointListWidth, int pointListHeight)
{
   _width = width;
   _height = height;
   _featuresWidth = featuresWidth;
   _featuresHeight = featuresHeight;
   _pointListWidth = pointListWidth;
   _pointListHeight = pointListHeight;

   _pyrCreatorA.allocate(width, height, nLevels, 1);
   _pyrCreatorB.allocate(width, height, nLevels, 1);

   if (_trackWithGain)
   {
      _trackerWithGain = new KLT_TrackerWithGain(_config.nIterations, _config.nLevels,
                                                 _config.levelSkip, _config.windowWidth);
      _trackerWithGain->allocate(width, height, featuresWidth, featuresHeight);
      _trackerWithGain->setBorderMargin(_config.trackBorderMargin);
      _trackerWithGain->setConvergenceThreshold(_config.convergenceThreshold);
      _trackerWithGain->setSSD_Threshold(_config.SSD_Threshold);
   }
   else
   {
      _tracker = new KLT_Tracker(_config.nIterations, _config.nLevels,
                                 _config.levelSkip, _config.windowWidth);
      _tracker->allocate(width, height, featuresWidth, featuresHeight);
      _tracker->setBorderMargin(_config.trackBorderMargin);
      _tracker->setConvergenceThreshold(_config.convergenceThreshold);
      _tracker->setSSD_Threshold(_config.SSD_Threshold);
   }
   _detector.allocate(width, height, pointListWidth, pointListHeight);

   _corners = new FeaturePoint[pointListWidth*pointListHeight];
} // end KLT_SequenceTracker::allocate()

void
KLT_SequenceTracker::deallocate()
{
   _pyrCreatorA.deallocate();
   _pyrCreatorB.deallocate();
   if (_tracker)
   {
      _tracker->deallocate();
      delete _tracker;
   }
   else
   {
      _trackerWithGain->deallocate();
      delete _trackerWithGain;
   }

   _detector.deallocate();

   delete [] _corners;
}

void
KLT_SequenceTracker::detect(unsigned char const * image,
                            int& nDetectedFeatures,
                            KLT_TrackedFeature * dest)
{
   int const nFeatures = _featuresWidth * _featuresHeight;

   nDetectedFeatures = 0;

   _pyrCreator1->buildPyramidForGrayscaleImage(image);

   _detector.detectCorners(_config.minCornerness, _pyrCreator1->textureID(), nDetectedFeatures);

   // To avoid buffer overflows.
   nDetectedFeatures = std::min(nDetectedFeatures, _pointListWidth*_pointListHeight);

   _detector.extractCorners(nDetectedFeatures, (float *)_corners);
   if (nDetectedFeatures > nFeatures)
   {
#if !defined(USE_PARTIAL_SORT)
      std::sort(_corners, _corners + nDetectedFeatures);
#else
      std::nth_element(_corners, _corners + nFeatures, _corners + nDetectedFeatures);
#endif
      nDetectedFeatures = nFeatures;
   }

#if defined(SHUFFLE_FEATURES)
   random_shuffle(_corners, _corners + nDetectedFeatures);
#endif

   if (_trackWithGain)
   {
      for (int i = 0; i < nFeatures; ++i)
         _corners[i].data[2] = 1.0f;
      _trackerWithGain->provideFeaturesAndGain((float *)_corners);
   }
   else
      _tracker->provideFeatures((float *)_corners);

   for (int i = 0; i < nDetectedFeatures; ++i)
   {
      dest[i].status = 1;
      dest[i].pos[0] = _corners[i].data[0];
      dest[i].pos[1] = _corners[i].data[1];
      dest[i].gain   = _corners[i].data[2];
   }

   for (int i = nDetectedFeatures; i < nFeatures; ++i) dest[i].status = -1;
} // end KLT_SequenceTracker::detect()

void
KLT_SequenceTracker::redetect(unsigned char const * image, int& nNewFeatures, KLT_TrackedFeature * dest)
{
   int const nFeatures = _featuresWidth * _featuresHeight;
   int nPresentFeatures = 0;

   // First, calculate still valid tracks.
   this->track(image, nPresentFeatures, dest);

   for (int i = 0; i < nFeatures; ++i)
   {
      if (dest[i].status >= 0)
      {
         _corners[i].data[0] = dest[i].pos[0];
         _corners[i].data[1] = dest[i].pos[1];
      }
      else
      {
         _corners[i].data[0] = -1.0f;
         _corners[i].data[1] = -1.0f;
      }
   } // end for (i)

   _detector.detectCorners(_config.minCornerness, _pyrCreator1->textureID(), nNewFeatures, nFeatures, (float *)_corners);
   // To avoid overflow during readback
   nNewFeatures = std::min(nNewFeatures, _pointListWidth*_pointListHeight);
   _detector.extractCorners(nNewFeatures, (float *)_corners);

   int const maxNewFeatures = nFeatures - nPresentFeatures;

   if (nNewFeatures > maxNewFeatures)
   {
      // Sort wrt. the cornerness value, take only the most distinctive corners.
#if !defined(USE_PARTIAL_SORT)
      std::sort(_corners, _corners + nNewFeatures);
#else
      std::nth_element(_corners, _corners + maxNewFeatures, _corners + nNewFeatures);
#endif
      nNewFeatures = maxNewFeatures;

#if defined(SHUFFLE_FEATURES)
      random_shuffle(_corners, _corners + nNewFeatures);
#endif
   }

   int k = 0;
   for (int i = 0; i < nFeatures && k < nNewFeatures; ++i)
   {
      if (dest[i].status < 0)
      {
         // Empty slot to fill.
         dest[i].status = 1;
         dest[i].pos[0] = _corners[k].data[0];
         dest[i].pos[1] = _corners[k].data[1];
         dest[i].gain   = _corners[k].data[2];
         ++k;
      }
   }

   for (int i = 0; i < nFeatures; ++i)
   {
      if (dest[i].status >= 0)
      {
         _corners[i].data[0] = dest[i].pos[0];
         _corners[i].data[1] = dest[i].pos[1];
         _corners[i].data[2] = 1.0f;
      }
      else
      {
         _corners[i].data[0] = -1.0f;
         _corners[i].data[1] = -1.0f;
         _corners[i].data[2] = 1.0f;
      }
   } // end for (i)

   if (_trackWithGain)
      _trackerWithGain->provideFeaturesAndGain((float *)_corners);
   else
      _tracker->provideFeatures((float *)_corners);
} // end KLT_SequenceTracker::redetect

void
KLT_SequenceTracker::track(unsigned char const * image, int& nPresentFeatures, KLT_TrackedFeature * dest)
{
   _pyrCreator1->buildPyramidForGrayscaleImage(image);

   int const nFeatures = _featuresWidth * _featuresHeight;

   nPresentFeatures = 0;

   if (_trackWithGain)
   {
      _trackerWithGain->trackFeaturesAndGain(_pyrCreator0->textureID(), _pyrCreator1->textureID());
      _trackerWithGain->readFeaturesAndGain((float *)_corners);
   }
   else
   {
      _tracker->trackFeatures(_pyrCreator0->textureID(), _pyrCreator1->textureID());
      _tracker->readFeatures((float *)_corners);
   }

   // Update point tracks.
   for (int i = 0; i < nFeatures; ++i)
   {
      float X = _corners[i].data[0];
      float Y = _corners[i].data[1];
      float gain = _corners[i].data[2];

      if (X >= 0)
      {
         dest[i].status = 0;
         dest[i].pos[0] = X;
         dest[i].pos[1] = Y;
         dest[i].gain   = gain;
         ++nPresentFeatures;
      }
      else
         dest[i].status = -1;
   } // end for (i)
} // end KLT_SequenceTracker::track()

#endif // defined(V3DLIB_GPGPU_ENABLE_CG)
