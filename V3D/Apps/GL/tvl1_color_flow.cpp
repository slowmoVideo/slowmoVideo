#include "Base/v3d_image.h"
#include "Base/v3d_imageprocessing.h"
#include "Base/v3d_timer.h"
#include "Base/v3d_utilities.h"
#include "GL/v3d_gpucolorflow.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

using namespace std;
using namespace V3D;
using namespace V3D_GPU;

//#define USE_LAB_COLORSPACE 1
#define USE_NEW_TVL1_FLOW 1

namespace
{

   inline void
   convertRGBImageToCIELab(Image<unsigned char> const& src, Image<float>& dst)
   {
      int const w = src.width();
      int const h = src.height();
      dst.resize(w, h, 3);

      Vector3f rgb, xyz, lab;

      for (int y = 0; y < h; ++y)
         for (int x = 0; x < w; ++x)
         {
            rgb[0] = src(x, y, 0) / 255.0f;
            rgb[1] = src(x, y, 1) / 255.0f;
            rgb[2] = src(x, y, 2) / 255.0f;

            rgb = convertRGBPixelTo_sRGB(rgb);
            xyz = convert_sRGBPixelToXYZ(rgb);
            lab = convertXYZPixelToCIELab(xyz);
            scaleVectorIP(0.01f, lab);

            dst(x, y, 0) = lab[0];
            dst(x, y, 1) = lab[1];
            dst(x, y, 2) = lab[2];
         }
   } // end convertRGBImageToCIELab()

   void displayTexture(unsigned textureID, float scale = 1.0f)
   {
      static Cg_FragmentProgram * shader = 0;

      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("::displayColor::shader");
         char const * source =
            "void main(uniform sampler2D uv_tex : TEXTURE0, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "          uniform float  scale, \n"
            "              out float4 color_out : COLOR0) \n"
            "{ \n"
            "   color_out.xyz = tex2D(uv_tex, st0).x / 255; \n"
            "   color_out.w = 1;"
            "} \n";
         shader->setProgram(source);
         shader->compile();
         checkGLErrorsHere0();
      }

      setupNormalizedProjection(true);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glEnable(GL_TEXTURE_2D);
      shader->parameter("scale", scale);
      shader->enable();
      renderNormalizedQuad();
      shader->disable();
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      checkGLErrorsHere0();
   } // end displayTexture()

   void displayResidual(unsigned textureID, float scale, int level = -1)
   {
      static Cg_FragmentProgram * shader = 0;

      if (shader == 0)
      {
         shader = new Cg_FragmentProgram("::displayResidual::shader");
         char const * source =
            "void main(uniform sampler2D uv_tex : TEXTURE0, \n"
            "                  float2 st0 : TEXCOORD0, \n"
            "          uniform float  scale, \n"
            "              out float4 color_out : COLOR0) \n"
            "{ \n"
            "   color_out.xyz = 1 - scale * tex2D(uv_tex, st0).w; \n"
            //"   color_out.xyz = scale * abs(tex2D(uv_tex, st0).z)/255; \n"
            "   color_out.w = 1;"
            "} \n";
         shader->setProgram(source);
         shader->compile();
         checkGLErrorsHere0();
      }

      setupNormalizedProjection(true);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      if (level >= 0) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);
      glEnable(GL_TEXTURE_2D);
      shader->parameter("scale", scale);
      shader->enable();
      renderNormalizedQuad();
      shader->disable();
      glActiveTexture(GL_TEXTURE0);
      if (level >= 0) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
      glDisable(GL_TEXTURE_2D);
      checkGLErrorsHere0();
   } // end displayResidual()

   int const nLevels = 6;
   int const startLevel = 0;
   int       nIterations = 200;
   int const nOuterIterations = 4;
   float const lambdaScale = 1.0;

   int win, scrwidth = 0, scrheight = 0;

   Image<unsigned char> leftImage, rightImage;
   Image<float> leftImageLab, rightImageLab;

   int const nTimingIterations = 1;
   float lambda = 1.0f;

#if !defined(USE_NEW_TVL1_FLOW)
   float const tau_primal = 0.5f;
   float const tau_dual   = 0.9f;
   typedef TVL1_ColorFlowEstimator_Direct TVL1_FlowEstimator;
   TVL1_FlowEstimator::Config flowCfg(tau_primal, tau_dual, lambdaScale);
#elif 0
   float const tau_primal = 0.5f;
   float const tau_dual   = 0.9f;
   typedef TVL1_ColorFlowEstimator_New TVL1_FlowEstimator;
   TVL1_FlowEstimator::Config flowCfg(tau_primal, tau_dual, lambdaScale);
#else
   float const tau   = 0.249f;
   float const theta = 0.1f;
   typedef TVL1_ColorFlowEstimator_QR TVL1_FlowEstimator;
   TVL1_FlowEstimator::Config flowCfg(tau, theta);
#endif

   TVL1_FlowEstimator * flowEstimator;

#if !defined(USE_LAB_COLORSPACE)
   PyramidWithDerivativesCreator leftPyrR(false), rightPyrR(false);
   PyramidWithDerivativesCreator leftPyrG(false), rightPyrG(false);
   PyramidWithDerivativesCreator leftPyrB(false), rightPyrB(false);
#else
   char const * pyrTexSpec = "r=32f noRTT";
   PyramidWithDerivativesCreator leftPyrR(false, pyrTexSpec), rightPyrR(false, pyrTexSpec);
   PyramidWithDerivativesCreator leftPyrG(false, pyrTexSpec), rightPyrG(false, pyrTexSpec);
   PyramidWithDerivativesCreator leftPyrB(false, pyrTexSpec), rightPyrB(false, pyrTexSpec);
#endif

   void
   reshape(int width, int height)
   {
      cout << "reshape" << endl;

      scrwidth = width;
      scrheight = height;
      glViewport(0, 0, (GLint) width, (GLint) height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(-10, 1010, 10, 1010);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
   }

   void
   drawscene()
   {
      static bool initialized = false;

      int const w = leftImage.width();
      int const h = leftImage.height();

      if (!initialized)
      {
         cout << "Start initialization..." << endl;

         glewInit();
         Cg_ProgramBase::initializeCg();

         flowEstimator = new TVL1_FlowEstimator(nLevels);
         flowEstimator->configurePrecision(false, false, false);
         //flowEstimator->configurePrecision(true, true, true);
         flowEstimator->allocate(w, h);
         flowEstimator->setLambda(lambda);
         flowEstimator->configure(flowCfg);
         flowEstimator->setInnerIterations(nIterations);
         flowEstimator->setOuterIterations(nOuterIterations);
         flowEstimator->setStartLevel(startLevel);

         leftPyrR.allocate(w, h, nLevels);
         rightPyrR.allocate(w, h, nLevels);
         leftPyrG.allocate(w, h, nLevels);
         rightPyrG.allocate(w, h, nLevels);
         leftPyrB.allocate(w, h, nLevels);
         rightPyrB.allocate(w, h, nLevels);

         initialized = true;
         cout << "done." << endl;
      } // end if (initialized)

#if !defined(USE_LAB_COLORSPACE)
      if (leftImage.numChannels() == 3)
      {
         leftPyrR.buildPyramidForGrayscaleImage(leftImage.begin(0));
         leftPyrG.buildPyramidForGrayscaleImage(leftImage.begin(1));
         leftPyrB.buildPyramidForGrayscaleImage(leftImage.begin(2));
      }
      else
      {
         leftPyrR.buildPyramidForGrayscaleImage(leftImage.begin(0));
         leftPyrG.buildPyramidForGrayscaleImage(leftImage.begin(0));
         leftPyrB.buildPyramidForGrayscaleImage(leftImage.begin(0));
      }
      if (rightImage.numChannels() == 3)
      {
         rightPyrR.buildPyramidForGrayscaleImage(rightImage.begin(0));
         rightPyrG.buildPyramidForGrayscaleImage(rightImage.begin(1));
         rightPyrB.buildPyramidForGrayscaleImage(rightImage.begin(2));
      }
      else
      {
         rightPyrR.buildPyramidForGrayscaleImage(rightImage.begin(0));
         rightPyrG.buildPyramidForGrayscaleImage(rightImage.begin(0));
         rightPyrB.buildPyramidForGrayscaleImage(rightImage.begin(0));
      }
#else
      leftPyrR.buildPyramidForGrayscaleImage(leftImageLab.begin(0));
      leftPyrG.buildPyramidForGrayscaleImage(leftImageLab.begin(1));
      leftPyrB.buildPyramidForGrayscaleImage(leftImageLab.begin(2));

      rightPyrR.buildPyramidForGrayscaleImage(rightImageLab.begin(0));
      rightPyrG.buildPyramidForGrayscaleImage(rightImageLab.begin(1));
      rightPyrB.buildPyramidForGrayscaleImage(rightImageLab.begin(2));
#endif

      unsigned int leftPyrTexIDs[3];
      unsigned int rightPyrTexIDs[3];

      leftPyrTexIDs[0] = leftPyrR.textureID();
      leftPyrTexIDs[1] = leftPyrG.textureID();
      leftPyrTexIDs[2] = leftPyrB.textureID();
      rightPyrTexIDs[0] = rightPyrR.textureID();
      rightPyrTexIDs[1] = rightPyrG.textureID();
      rightPyrTexIDs[2] = rightPyrB.textureID();

      Timer t;
      for (int k = 0; k < nTimingIterations; ++k)
      {
         t.start();
         flowEstimator->run(leftPyrTexIDs, rightPyrTexIDs);
         glFinish();
         t.stop();
      }
      t.print();

      //flowEstimator->computeAlternateFlow();

      warpImageWithFlowField(flowEstimator->getFlowFieldTextureID(),
                             leftPyrG.textureID(), rightPyrG.textureID(), startLevel,
                             *flowEstimator->getWarpedBuffer(1, startLevel));

      FrameBufferObject::disableFBORendering();
      glViewport(0, 0, scrwidth/2, scrheight);
      setupNormalizedProjection(true);

      glColor3f(1, 1, 1);
      glActiveTexture(GL_TEXTURE0_ARB);
      //glBindTexture(GL_TEXTURE_2D, leftPyr.sourceTextureID());
      //glEnable(GL_TEXTURE_2D);
      //renderNormalizedQuad();
      //glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, leftPyrG.textureID());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, startLevel);
      displayTexture(leftPyrG.textureID());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

      displayResidual(flowEstimator->getWarpedBuffer(1, startLevel)->textureID(), 1.0f/32);

      glViewport(scrwidth/2, 0, scrwidth/2, scrheight);
      // scale = 0.4 for Ettlinger tor
      //float const scale = 0.4f;
      //float const scale = 0.25f;
      //float const scale = 0.15f;
      //float const scale = 0.21414f; // Dimetrodon
      //float const scale = 0.5f; // Dimetrodon (d)
      //float const scale = 0.25f; // Grove2 (d), Army (l)
      //float const scale = 0.02f; // Walking
      float const scale = 0.01f; // Walking
      //float const scale = 0.1f; // Walking
      //float const scale = 0.2f; // Pentagon
      //float const scale = 0.5f; // Rubberwhale
      //float const scale = 0.2f; // Hydrangea
      //float const scale = 0.05f; // Urban2
      displayMotionAsColorLight(flowEstimator->getFlowFieldTextureID(), scale * (1 << startLevel), false);
      //displayMotionAsColorLight(flowEstimator->getFlowFieldTextureID(), sqrtf(scale) * (1 << startLevel), true);

      //displayMotionAsColorDark(flowEstimator->getFlowFieldTextureID(), scale * (1 << startLevel));
      //displayMotionAsColorDark(flowEstimator->getFlowFieldTextureID(), sqrtf(scale) * (1 << startLevel));

      //displayTexture(flowEstimator->getFlowFieldTextureID(), 0.01f);
      //displayTexture(flowEstimator->getWarpedBuffer(0)->textureID(), 0.25f);
      //displayResidual(flowEstimator->getWarpedBuffer(startLevel)->textureID(), 8.0f);

//       glActiveTexture(GL_TEXTURE0);
//       glBindTexture(GL_TEXTURE_2D, leftPyr.textureID());
//       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 2);
//       displayTexture(leftPyr.textureID(), 1.0f/255);
//       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

      {
         Image<unsigned char> flowIm(scrwidth/2, scrheight, 3);
         glReadPixels(scrwidth/2, 0, scrwidth/2, scrheight, GL_RED, GL_UNSIGNED_BYTE, &flowIm(0, 0, 0));
         glReadPixels(scrwidth/2, 0, scrwidth/2, scrheight, GL_GREEN, GL_UNSIGNED_BYTE, &flowIm(0, 0, 1));
         glReadPixels(scrwidth/2, 0, scrwidth/2, scrheight, GL_BLUE, GL_UNSIGNED_BYTE, &flowIm(0, 0, 2));

         flipImageUpsideDown(flowIm);

         saveImageFile(flowIm, "color_flow_tvl1_GL.png");
      }

      glutSwapBuffers();
   } // end drawscene()

   void
   keyFunc(unsigned char key, int x, int y)
   {
      if (key == 27) exit(0);
      switch (key)
      {
         case ' ':
            break;
      }
      glutPostRedisplay();
   }

} // end namespace <>

int
main( int argc, char** argv) 
{
   unsigned int win;

   //int const W = 320; int const H = 240;
   //int const W = 640; int const H = 480;
   //int const W = 512; int const H = 512;
   //int const W = 512; int const H = 384;
   //int const W = 584; int const H = 388;
   //int const W = 420; int const H = 380; // Venus
   //int const W = 960; int const H = 544;
   //int const W = 480; int const H = 272;
   //int const W = 824/2; int const H = 648/2;
   //int const W = 480; int const H = 720;
   int const W = 640; int const H = 480;

   glutInitWindowPosition(0, 0);
   glutInitWindowSize(2*W, H);
   glutInit(&argc, argv);

   if (argc != 4 && argc != 5)
   {
      cout << "Usage: " << argv[0] << " <left image> <right image> <lambda> [<nIterations>]" << endl;
      return -1;
   }

   loadImageFile(argv[1], leftImage);
   loadImageFile(argv[2], rightImage);
   lambda = atof(argv[3]);

   cout << "leftImage.numChannels() = " << leftImage.numChannels() << endl;
   cout << "rightImage.numChannels() = " << rightImage.numChannels() << endl;

#if !defined(USE_LAB_COLORSPACE)
   if (leftImage.numChannels() < 3 || rightImage.numChannels() < 3)
      cerr << "Warning: grayscale images provided." << endl;
#else
   if (leftImage.numChannels() < 3 || rightImage.numChannels() < 3)
   {
      cerr << "Error: grayscale images provided." << endl;
      return -2;
   }

   convertRGBImageToCIELab(leftImage, leftImageLab);
   convertRGBImageToCIELab(rightImage, rightImageLab);

   float maxL = -1e30f, minL = 1e30f;
   float maxA = -1e30f, minA = 1e30f;
   float maxB = -1e30f, minB = 1e30f;
   maxL = std::max(maxL, *max_element(leftImageLab.begin(0), leftImageLab.end(0)));
   minL = std::min(minL, *min_element(leftImageLab.begin(0), leftImageLab.end(0)));
   maxL = std::max(maxL, *max_element(rightImageLab.begin(0), rightImageLab.end(0)));
   minL = std::min(minL, *min_element(rightImageLab.begin(0), rightImageLab.end(0)));
   cout << "minL = " << minL << " maxL = " << maxL << endl;

   maxA = std::max(maxA, *max_element(leftImageLab.begin(1), leftImageLab.end(1)));
   minA = std::min(minA, *min_element(leftImageLab.begin(1), leftImageLab.end(1)));
   maxA = std::max(maxA, *max_element(rightImageLab.begin(1), rightImageLab.end(1)));
   minA = std::min(minA, *min_element(rightImageLab.begin(1), rightImageLab.end(1)));
   cout << "minA = " << minA << " maxA = " << maxA << endl;

   maxB = std::max(maxB, *max_element(leftImageLab.begin(2), leftImageLab.end(2)));
   minB = std::min(minB, *min_element(leftImageLab.begin(2), leftImageLab.end(2)));
   maxB = std::max(maxB, *max_element(rightImageLab.begin(2), rightImageLab.end(2)));
   minB = std::min(minB, *min_element(rightImageLab.begin(2), rightImageLab.end(2)));
   cout << "minB = " << minB << " maxB = " << maxB << endl;
#endif

   if (argc == 5) nIterations = atoi(argv[4]);

   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

   if (!(win = glutCreateWindow("GPU TV-L1 Optic Flow")))
   {
      cerr << "Error, couldn't open window" << endl;
      return -1;
   }

   glutReshapeFunc(reshape);
   glutDisplayFunc(drawscene);
   //glutIdleFunc(drawscene);
   glutKeyboardFunc(keyFunc);
   glutMainLoop();

   return 0;
}
