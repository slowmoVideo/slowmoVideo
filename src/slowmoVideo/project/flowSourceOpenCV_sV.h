/*
This file is part of slowmoVideo.
Copyright (C) 2012  Lucas Walter
              2012  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef FLOWSOURCEOPENCV_SV_H
#define FLOWSOURCEOPENCV_SV_H


#include "abstractFlowSource_sV.h"

#include <QtCore/QDir>

#include "opencv2/core/version.hpp"

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#if CV_MAJOR_VERSION == 2
// do opencv 2 code
// now in core (issue #32) 
#include "opencv2/core/gpumat.hpp"

#include "opencv2/ocl/ocl.hpp"
//#elif CV_MAJOR_VERSION == 3
// do opencv 3 code
#endif


class FlowSourceOpenCV_sV : public AbstractFlowSource_sV
{
 
    
public:
    FlowSourceOpenCV_sV(Project_sV *project);
    ~FlowSourceOpenCV_sV() {}

    virtual FlowField_sV* buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError);
    virtual const QString flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize = FrameSize_Orig) const;

    virtual void buildFlowForwardCache(FrameSize frameSize) throw(FlowBuildingError);
	
    void setupOpticalFlow(const int levels,const int winsize,const double polySigma, const double pyrScale, const int polyN);
    void setupTVL(double thau,double lambda, double pyrScale, double warp);

    void initGPUDevice(int dev);
    void chooseAlgo(int algo);
    
  
private:
	int use_gpu;
	int method;
	
    // optical flow Farn
   	int numLevels;
	  int numIters;
	  int winSize;
	  double polySigma;
	  double pyrScale;
	  int polyN;
	  int flags;

	  // optical TVL1
};

#endif // FLOWSOURCEOPENCV_SV_H
