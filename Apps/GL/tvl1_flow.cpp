/*
Copyright (c) 2008-2010 UNC-Chapel Hill & ETH Zurich

This file is part of GPU-KLT+FLOW.

GPU-KLT+FLOW is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

GPU-KLT+FLOW is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License along
with GPU-KLT+FLOW. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Base/v3d_image.h"
#include "Base/v3d_timer.h"
#include "Base/v3d_utilities.h"
#include "GL/v3d_gpuflow.h"
#include "GL/v3d_gpupyramid.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

using namespace std;
using namespace V3D;
using namespace V3D_GPU;

namespace
{

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

   int const nLevels = 4;
   int const startLevel = 0;
   int       nIterations = 200;
   int const nOuterIterations = 10;
   float const theta = 0.02f;
   float const beta = 0.0f;
   float const lambdaScale = 1.0;

   int win, scrwidth = 0, scrheight = 0;

   Image<unsigned char> leftImage, rightImage;

   int const nTimingIterations = 1;
   float lambda = 1.0f;

   float const tau = 0.249f;
   typedef TVL1_FlowEstimator_Relaxed TVL1_FlowEstimator;
   TVL1_FlowEstimator::Config flowCfg(tau, theta);

   TVL1_FlowEstimator * flowEstimator;

   PyramidWithDerivativesCreator leftPyr(false), rightPyr(false);

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

         cout << "left pyr allocate..." << endl;
         leftPyr.allocate(w, h, nLevels);
         cout << "right pyr allocate..." << endl;
         rightPyr.allocate(w, h, nLevels);

         initialized = true;
         cout << "done." << endl;
      } // end if (initialized)

      leftPyr.buildPyramidForGrayscaleImage(&leftImage(0, 0));
      rightPyr.buildPyramidForGrayscaleImage(&rightImage(0, 0));

      Timer t;
      for (int k = 0; k < nTimingIterations; ++k)
      {
         t.start();
         flowEstimator->run(leftPyr.textureID(), rightPyr.textureID());
         glFinish();
         t.stop();
      }
      t.print();

      //flowEstimator->computeAlternateFlow();

      warpImageWithFlowField(flowEstimator->getFlowFieldTextureID(),
                             leftPyr.textureID(), rightPyr.textureID(), startLevel,
                             *flowEstimator->getWarpedBuffer(startLevel));

      FrameBufferObject::disableFBORendering();
      glViewport(0, 0, scrwidth/2, scrheight);
      setupNormalizedProjection(true);

      glColor3f(1, 1, 1);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, leftPyr.textureID());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, startLevel);
      displayTexture(leftPyr.textureID());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

      displayResidual(flowEstimator->getWarpedBuffer(startLevel)->textureID(), 1.0f/32);

      glViewport(scrwidth/2, 0, scrwidth/2, scrheight);
      // scale = 0.4 for Ettlinger tor
      //float const scale = 0.4f;
      //float const scale = 0.25f;
      //float const scale = 0.15f;
      //float const scale = 0.21414f; // Dimetrodon
      float const scale = 0.1f; // Walking
      //float const scale = 0.2f; // Pentagon
      //displayMotionAsColor(flowEstimator->getFlowFieldTextureID(), scale * (1 << startLevel), false);
      displayMotionAsColorLight(flowEstimator->getFlowFieldTextureID(), sqrtf(scale) * (1 << startLevel), true);

      //displayMotionAsColorDark(flowEstimator->getFlowFieldTextureID(), sqrtf(scale) * (1 << startLevel));
      //displayMotionAsColorDark(flowEstimator->getAlternateFlowFieldTextureID(), sqrtf(scale) * (1 << startLevel));

      //displayTexture(flowEstimator->getFlowFieldTextureID(), 0.01f);
      //displayTexture(flowEstimator->getWarpedBuffer(0)->textureID(), 0.25f);
      //displayResidual(flowEstimator->getWarpedBuffer(startLevel)->textureID(), 8.0f);

      {
         Image<unsigned char> flowIm(scrwidth/2, scrheight, 3);
         glReadPixels(scrwidth/2, 0, scrwidth/2, scrheight, GL_RED, GL_UNSIGNED_BYTE, &flowIm(0, 0, 0));
         glReadPixels(scrwidth/2, 0, scrwidth/2, scrheight, GL_GREEN, GL_UNSIGNED_BYTE, &flowIm(0, 0, 1));
         glReadPixels(scrwidth/2, 0, scrwidth/2, scrheight, GL_BLUE, GL_UNSIGNED_BYTE, &flowIm(0, 0, 2));

         flipImageUpsideDown(flowIm);

         saveImageFile(flowIm, "flow_tvl1_GL.png");
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
   int const W = 584; int const H = 388;
   //int const W = 420; int const H = 380; // Venus

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
