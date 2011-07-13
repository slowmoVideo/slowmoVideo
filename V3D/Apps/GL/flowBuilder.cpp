/// \todo Include shader files in binary
/// \todo Check that (width|height)/2^nLevels >= 1

#include "Base/v3d_image.h"
#include "Base/v3d_imageprocessing.h"
#include "Base/v3d_timer.h"
#include "Base/v3d_utilities.h"
#include "GL/v3d_gpucolorflow.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

#include "flowRW_sV.h"
#include "flowField_sV.h"

using namespace V3D;
using namespace V3D_GPU;

#define VERSION "1.0"

//#define USE_LAB_COLORSPACE 1
#define USE_NEW_TVL1_FLOW 1

int const nLevels = 6;  // Must be large enough for big images. Number of pyramid levels.
int const startLevel = 0;
int       nIterations = 200;
int const nOuterIterations = 4;
float const lambdaScale = 1.0;

Image<unsigned char> leftImage, rightImage;
Image<float> leftImageLab, rightImageLab;
const char *outputFile;

float lambda = 1.0f;

float const tau   = 0.249f;
float const theta = 0.1f;
typedef TVL1_ColorFlowEstimator_QR TVL1_FlowEstimator;
TVL1_FlowEstimator::Config flowCfg(tau, theta);

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

inline void convertRGBImageToCIELab(Image<unsigned char> const& src, Image<float>& dst)
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


void drawscene()
   {
      int const w = leftImage.width();
      int const h = leftImage.height();

     std::cout << "Start initialization..." << std::endl;
     glewInit();
     Cg_ProgramBase::initializeCg();

     flowEstimator = new TVL1_FlowEstimator(nLevels);
     flowEstimator->configurePrecision(false, false, false);
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
     std::cout << "done." << std::endl;

#if !defined(USE_LAB_COLORSPACE)
      if (leftImage.numChannels() == 3) {
         leftPyrR.buildPyramidForGrayscaleImage(leftImage.begin(0));
         leftPyrG.buildPyramidForGrayscaleImage(leftImage.begin(1));
         leftPyrB.buildPyramidForGrayscaleImage(leftImage.begin(2));
      } else {
         leftPyrR.buildPyramidForGrayscaleImage(leftImage.begin(0));
         leftPyrG.buildPyramidForGrayscaleImage(leftImage.begin(0));
         leftPyrB.buildPyramidForGrayscaleImage(leftImage.begin(0));
      }
      if (rightImage.numChannels() == 3) {
         rightPyrR.buildPyramidForGrayscaleImage(rightImage.begin(0));
         rightPyrG.buildPyramidForGrayscaleImage(rightImage.begin(1));
         rightPyrB.buildPyramidForGrayscaleImage(rightImage.begin(2));
      } else {
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

      flowEstimator->run(leftPyrTexIDs, rightPyrTexIDs);

      warpImageWithFlowField(flowEstimator->getFlowFieldTextureID(),
                             leftPyrG.textureID(), rightPyrG.textureID(), startLevel,
                             *flowEstimator->getWarpedBuffer(1, startLevel));


      // Save the generated flow field
      float *data = new float[3*leftImage.width()*leftImage.height()];

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, flowEstimator->getFlowFieldTextureID());
//      glEnable(GL_TEXTURE_2D); // Not required for reading
//      glReadPixels(0, 0, leftImage.width(), leftImage.height(), GL_RGB, GL_FLOAT, data); // Wrong, reads frame buffer
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);

      FlowField_sV field(leftImage.width(), leftImage.height(), data, FlowField_sV::GLFormat_RGB);
      FlowRW_sV::save(outputFile, &field);
      std::cout << "Flow data written to " << outputFile << "." << std::endl;

      delete[] data;

      exit(0);
   }

int main( int argc, char** argv)
{

    if ((argc-1) == 1) {
        if (strcmp(argv[1], "--identify") == 0) {
            std::cout << "flowBuilder v" << VERSION << std::endl;
            return 0;
        }
    }

   if ((argc-1) < 3) {
       std::cout << "Usage: " << argv[0] << " <left image> <right image> <outFilename> "
               "[ <lambda=" << lambda << "> [<nIterations=" << nIterations << ">] ]" << std::endl;
       return -1;
   }
#ifdef INCLUDE_SOURCE
   std::cout << "Using internal shader files." << std::endl;
#else
   if (getenv("V3D_SHADER_DIR") == NULL) {
       std::cout << "V3D_SHADER_DIR environment variable needs to be set!";
       return -2;
   }
#endif

   loadImageFile(argv[1], leftImage);
   loadImageFile(argv[2], rightImage);
   outputFile = argv[3];

   if ((argc-1) >= 4) {
       lambda = atof(argv[4]);
       if ((argc-1) >= 5) {
           nIterations = atoi(argv[5]);
       }
   }

   std::cout << "leftImage.numChannels() = " << leftImage.numChannels() << std::endl;
   std::cout << "rightImage.numChannels() = " << rightImage.numChannels() << std::endl;

   glutInitWindowPosition(0, 0);
   glutInitWindowSize(100, 100);
   glutInit(&argc, argv);

#if !defined(USE_LAB_COLORSPACE)
   if (leftImage.numChannels() < 3 || rightImage.numChannels() < 3)
      cerr << "Warning: grayscale images provided." << std::endl;
#else
   if (leftImage.numChannels() < 3 || rightImage.numChannels() < 3)
   {
      cerr << "Error: grayscale images provided." << std::endl;
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
   std::cout << "minL = " << minL << " maxL = " << maxL << std::endl;

   maxA = std::max(maxA, *max_element(leftImageLab.begin(1), leftImageLab.end(1)));
   minA = std::min(minA, *min_element(leftImageLab.begin(1), leftImageLab.end(1)));
   maxA = std::max(maxA, *max_element(rightImageLab.begin(1), rightImageLab.end(1)));
   minA = std::min(minA, *min_element(rightImageLab.begin(1), rightImageLab.end(1)));
   std::cout << "minA = " << minA << " maxA = " << maxA << std::endl;

   maxB = std::max(maxB, *max_element(leftImageLab.begin(2), leftImageLab.end(2)));
   minB = std::min(minB, *min_element(leftImageLab.begin(2), leftImageLab.end(2)));
   maxB = std::max(maxB, *max_element(rightImageLab.begin(2), rightImageLab.end(2)));
   minB = std::min(minB, *min_element(rightImageLab.begin(2), rightImageLab.end(2)));
   std::cout << "minB = " << minB << " maxB = " << maxB << std::endl;
#endif

   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

   if (!glutCreateWindow("GPU TV-L1 Optic Flow")) {
      cerr << "Error, couldn't open window" << std::endl;
      return -1;
   }

   glutDisplayFunc(drawscene);
   glutMainLoop();

   return 0;
}
