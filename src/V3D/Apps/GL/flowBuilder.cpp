/// \todo Check that (width|height)/2^nLevels >= 1

#include "Base/v3d_image.h"
#include "Base/v3d_imageprocessing.h"
#include "Base/v3d_timer.h"

#include "GL/v3d_gpucolorflow.h"

#include <iostream>


#include <GL/glew.h>
#ifdef __APPLE__
#include <glut.h>

#include <OpenGL/gl.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>

#elif defined(_WIN32)
#include <GL/glut.h>
#else
#define USE_RAW_X11
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#endif

#include "flowRW_sV.h"
#include "flowField_sV.h"

using namespace V3D;
using namespace V3D_GPU;

#define VERSION "2.0"

//#define USE_LAB_COLORSPACE 1

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

#ifdef USE_LAB_COLORSPACE
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
#endif

void drawscene()
{
  int const w = leftImage.width();
  int const h = leftImage.height();
   
  {
    ScopedTimer st("glew/cg init"); 
    glewInit();
  }
  {
    ScopedTimer st("initialization flow"); 
	
    flowEstimator = new TVL1_FlowEstimator(nLevels);
    flowEstimator->configurePrecision(false, false, false);
    flowEstimator->allocate(w, h);
    flowEstimator->setLambda(lambda);
    flowEstimator->configure(flowCfg);
    flowEstimator->setInnerIterations(nIterations);
    flowEstimator->setOuterIterations(nOuterIterations);
    flowEstimator->setStartLevel(startLevel);
  }
  {
    ScopedTimer st("allocating pyramids"); 

    leftPyrR.allocate(w, h, nLevels);
    rightPyrR.allocate(w, h, nLevels);
    leftPyrG.allocate(w, h, nLevels);
    rightPyrG.allocate(w, h, nLevels);
    leftPyrB.allocate(w, h, nLevels);
    rightPyrB.allocate(w, h, nLevels);
  }

  unsigned int leftPyrTexIDs[3];
  unsigned int rightPyrTexIDs[3];

  {
    ScopedTimer st("building pyramids"); 
       
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
	
    leftPyrTexIDs[0] = leftPyrR.textureID();
    leftPyrTexIDs[1] = leftPyrG.textureID();
    leftPyrTexIDs[2] = leftPyrB.textureID();
    rightPyrTexIDs[0] = rightPyrR.textureID();
    rightPyrTexIDs[1] = rightPyrG.textureID();
    rightPyrTexIDs[2] = rightPyrB.textureID();
  }
   
  {
    ScopedTimer st("flowEstimator"); 	
    flowEstimator->run(leftPyrTexIDs, rightPyrTexIDs);
  }

  // Save the generated flow field
  float *data = new float[2*leftImage.width()*leftImage.height()];	
  {
    ScopedTimer st("readPixels"); 
    RTT_Buffer *buf = flowEstimator->getFlowBuffer(); 
    buf->makeCurrent(); 
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0,0,leftImage.width(), leftImage.height(), GL_RG, GL_FLOAT, data); 
  }
  
  { 
    ScopedTimer st("saving"); 
    FlowField_sV field(leftImage.width(), leftImage.height(), data, FlowField_sV::GLFormat_RG);
    FlowRW_sV::save(outputFile, &field);   
  }
  exit(0);
}

int main( int argc, char** argv)
{
   if ((argc-1) == 1) {
     if (strcmp(argv[1], "--identify") == 0) {
            std::cout << "slowmoFlowBuilder v" << VERSION << std::endl;
            return 0;
        }
    }

   if ((argc-1) < 3) {
       std::cout << "Usage: " << argv[0] << " <left image> <right image> <outFilename> "
               "[ <lambda=" << lambda << "> [<nIterations=" << nIterations << ">] ]" << std::endl;
       return -1;
   }
   
   {
     ScopedTimer st("loading files"); 
     loadImageFile(argv[1], leftImage);
     loadImageFile(argv[2], rightImage);
   }
   
   outputFile = argv[3];
	
   if ((argc-1) >= 4) {
       lambda = atof(argv[4]);
       if ((argc-1) >= 5) {
           nIterations = atoi(argv[5]);
       }
   }

   if (leftImage.numChannels() != 3 || rightImage.numChannels() != 3) {
        std::cout << "leftImage.numChannels() = " << leftImage.numChannels() << std::endl;
        std::cout << "rightImage.numChannels() = " << rightImage.numChannels() << std::endl;
   }

   {
     ScopedTimer st("initialization GL"); 
#ifdef USE_RAW_X11
     Display *dpy = XOpenDisplay(0); 
     if (!dpy) {
       std::cerr << "ERROR: could not open display\n"; 
       exit(1); 
     }

     GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None, 0 };
     XVisualInfo *vi = glXChooseVisual(dpy,0,att);
     if (!vi) {
       std::cerr << "ERROR: could not choose a GLX visual\n"; 
       exit(1); 
     }

     Colormap cmap = XCreateColormap(dpy, DefaultRootWindow(dpy), vi->visual, AllocNone);
     XSetWindowAttributes wattr; 
     wattr.colormap = cmap; 
     
     Window win = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0, vi->depth, InputOutput, vi->visual,CWColormap, &wattr);
     if (win == None) {
       std::cerr << "ERROR: could not create window\n"; 
       exit(1); 
     }
     GLXContext ctx = glXCreateContext(dpy, vi, NULL, GL_TRUE);
     if (!ctx) {
       std::cerr << "ERROR: could not create GL context\n"; 
       exit(1); 
     }

     if (!glXMakeCurrent(dpy,win,ctx)) {
       std::cerr << "ERROR: could not make context current\n"; 
       exit(1);     
     }
#else
     glutInitWindowPosition(0, 0);
     glutInitWindowSize(100, 100);
     glutInit(&argc, argv);
#endif
   }

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

#ifdef USE_RAW_X11
   drawscene(); 
#else
   //glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
//   glutInitContextVersion(3,2); /* or later versions, core was introduced only with 3.2 */
//   glutInitContextProfile(GLUT_CORE_PROFILE);


   if (!glutCreateWindow("GPU TV-L1 Optic Flow")) {
      cerr << "Error, couldn't open window" << std::endl;
      return -1;
   }

#ifdef __APPLE__
   CGLContextObj ctx = CGLGetCurrentContext();
   char *vendor = (char*)glGetString(GL_VENDOR);
   char *renderer = (char*)glGetString(GL_RENDERER);
   char *version = (char*)glGetString(GL_VERSION);
   printf("vendor: %s\n",vendor);
   fprintf(stderr,"%s\n%s\n", 
        renderer,  // e.g. Intel HD Graphics 3000 OpenGL Engine
        version    // e.g. 3.2 INTEL-8.0.61
        );

//   CGLSetVirtualScreen(ctx, 1); // second GPU ?
//   vendor = (char*)glGetString(GL_VENDOR);
//   printf("vendor: %s\n",vendor);
#endif

   glutDisplayFunc(drawscene);
   glutMainLoop();
#endif

   return 0;
}
