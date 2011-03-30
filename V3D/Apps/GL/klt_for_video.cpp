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

#include "GL/v3d_gpupyramid.h"
#include "GL/v3d_gpuklt.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <iostream>
#include <cstdio>
#include <vector>

#include <GL/glew.h>
#include <GL/glut.h>

using namespace std;
using namespace V3D_GPU;

//#define PREFETCH_VIDEO 1

namespace
{

   struct PointTrack
   {
         PointTrack() : len(0) { }

         void add(float X, float Y)
         {
            pos[len][0] = X;
            pos[len][1] = Y;
            ++len;
         }

         void clear()
         {
            len = 0;
         }

         bool isValid() const { return pos[len-1][0] >= 0; }

         int len;
         float pos[4096][2];
   }; // end struct PointTrack

   CvCapture * capture = 0;
   int width, height;

   bool      trackWithGain = false;
   int const featuresWidth = 32;
   int const featuresHeight = 32;
   unsigned int const nFeatures = featuresWidth*featuresHeight;
   int const nTrackedFrames = 10;
   int const nTimedFrames = 400;
   int const nLevels = 4;
   int const pointListWidth = 64;
   int const pointListHeight = 64;

   int win, scrwidth, scrheight;
   bool initialized = false;

   KLT_SequenceTracker * tracker = 0;
   vector<IplImage *>    allFrames;
   vector<float>         trueGains;

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

   void done();

   void
   drawscene()
   {
      static int frameNo = 0;

      static int nDetectedFeatures = 0;
      static IplImage * videoFrame;
      static PointTrack * tracks;
      static KLT_TrackedFeature * features = 0;

      if (!initialized)
      {
         CvSize sz = cvSize(width, height);
         videoFrame = cvCreateImage(sz, 8, 1);

         features = new KLT_TrackedFeature[nFeatures];
         tracks = new PointTrack[nFeatures];

         glewInit();
         Cg_ProgramBase::initializeCg();

         KLT_SequenceTrackerConfig cfg;
         cfg.nIterations = 10;
         cfg.nLevels     = nLevels;
         cfg.levelSkip   = 1;
         cfg.trackBorderMargin    = 20.0f;
         cfg.convergenceThreshold = 0.1f;
         cfg.SSD_Threshold        = 1000.0f;
         cfg.trackWithGain        = true;

         cfg.minDistance   = 10;
         cfg.minCornerness = 50.0f;
         cfg.detectBorderMargin = cfg.trackBorderMargin;

         tracker = new KLT_SequenceTracker(cfg);
         tracker->allocate(width, height, nLevels, featuresWidth, featuresHeight);

         initialized = true;
         cout << "Done with initialization." << endl;
      }

#if !defined(PREFETCH_VIDEO)
      IplImage * im = cvQueryFrame(capture);
      if (im)
         cvCvtColor(im, videoFrame, CV_RGB2GRAY);
      else
      {
         cerr << "Could not read frame from video source. Exiting..." << endl;
         done();
      }
#else
      //if (frameNo >= allFrames.size()) done();
      videoFrame = allFrames[frameNo % allFrames.size()];
#endif

      int const relFrameNo = (frameNo % nTrackedFrames);

      if (frameNo == 0)
      {
         tracker->detect((V3D_GPU::uchar *)videoFrame->imageData, nDetectedFeatures, features);
         cout << "nDetectedFeatures = " << nDetectedFeatures << endl;
      }
      else if (relFrameNo == 0)
      {
         int nNewFeatures;
         tracker->redetect((V3D_GPU::uchar *)videoFrame->imageData, nNewFeatures, features);
         cout << "nNewFeatures = " << nNewFeatures << endl;
      }
      else
      {
         int nPresentFeatures;
         tracker->track((V3D_GPU::uchar *)videoFrame->imageData, nPresentFeatures, features);
         //cout << "nPresentFeatures = " << nPresentFeatures << endl;
      }

      int nTracks = 0;
      for (int i = 0; i < nFeatures; ++i)
      {
         if (features[i].status == 0)
         {
            tracks[i].add(features[i].pos[0], features[i].pos[1]);
            ++nTracks;
         }
         else if (features[i].status > 0)
         {
            tracks[i].len = 1;
            tracks[i].pos[0][0] = features[i].pos[0];
            tracks[i].pos[0][1] = features[i].pos[1];
         }
         else
            tracks[i].clear();
      } // end for (i)

      //cout << "nTracks = " << nTracks << endl;

      glFinish();

      // Draw texture.
      FrameBufferObject::disableFBORendering();
      glViewport(0, 0, scrwidth, scrheight);
      setupNormalizedProjection(true);

      glColor3f(1, 1, 1);
      glActiveTexture(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, tracker->getCurrentFrameTextureID());
      glEnable(GL_TEXTURE_2D);
      renderNormalizedQuad();
      glDisable(GL_TEXTURE_2D);

#if 1
      // Draw lines.
      glColor3f(1, 0.3, 0.2);
      for (int i = 0; i < nFeatures; ++i)
      {
         if (tracks[i].len > 1 && tracks[i].isValid())
         {
            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < tracks[i].len; ++j)
               glVertex2fv(tracks[i].pos[j]);
            glEnd();
         }
      } // end for (i)

      // Draw newly created corners.
      glPointSize(5);
      glColor3f(0.2, 0.3, 1.0);
      glBegin(GL_POINTS);
      for (int i = 0; i < nFeatures; ++i)
      {
         if (features[i].status > 0)
            glVertex2fv(features[i].pos);
      } // end for (i)
      glEnd();

      glColor3f(0.2, 1.0, 0.3);
      glBegin(GL_POINTS);
      for (int i = 0; i < nFeatures; ++i)
      {
         if (features[i].status == 0)
            glVertex2fv(features[i].pos);
      } // end for (i)
      glEnd();
#endif

      tracker->advanceFrame();
      ++frameNo;

      glutSwapBuffers();
   } // end drawscene()

   void
   done()
   {
      FrameBufferObject::disableFBORendering();
      tracker->deallocate();
      exit(0);
   }

   void
   keyFunc(unsigned char key, int x, int y)
   {
      if (key == 27) done();
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

   glutInitWindowPosition(0, 0);
   glutInitWindowSize(640, 480);
   glutInit(&argc, argv);

   if (argc != 3)
   {
      cout << "Usage: " << argv[0] << " <video> <trackWithGain>" << endl;
      return -1;
   }

   capture = cvCreateFileCapture(argv[1]);

   trackWithGain = atoi(argv[2]);

   width  = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
   height = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);

   cout << "w = " << width << ", h = " << height << endl;

#if defined(PREFETCH_VIDEO)
   cout << "Reading all frames from the video..." << endl;
   allFrames.reserve(1000);
   {
      CvSize sz = cvSize(width, height);

      for (;;)
      {
         IplImage * im = cvQueryFrame(capture);
         if (im)
         {
            IplImage * im1 = cvCreateImage(sz, 8, 1);
            cvCvtColor(im, im1, CV_RGB2GRAY);
            allFrames.push_back(im1);
         }
         else
            break;
      }
      cout << "allFrames.size() = " << allFrames.size() << endl;
   }
#endif

   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

   if (!(win = glutCreateWindow("GPU KLT Test")))
   {
      cerr << "Error, couldn't open window" << endl;
      return -1;
   }

   glutReshapeFunc(reshape);
   glutDisplayFunc(drawscene);
   glutIdleFunc(drawscene);
   glutKeyboardFunc(keyFunc);
   glutMainLoop();

   return 0;
}
