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
#include "opencv2/opencv_modules.hpp"

#if CV_MAJOR_VERSION == 2
#include "opencv2/core/gpumat.hpp"

#ifdef HAVE_OPENCV_OCL
#include "opencv2/ocl/ocl.hpp"
#endif

#else
#include "opencv2/core/ocl.hpp"
#endif


class FlowSourceOpenCV_sV : public AbstractFlowSource_sV
{

public:
    FlowSourceOpenCV_sV(Project_sV *project, int algo, int ocl_dev_index);
    ~FlowSourceOpenCV_sV() {}

    virtual FlowField_sV* buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError);
    virtual const QString flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize = FrameSize_Orig) const;

    void setupOpticalFlow(const int levels,const int winsize,const double polySigma, const double pyrScale, const int polyN);
    void setupTVL1(const double tau, const double lambda, const int nscales, const int warps, const int iterations, const double epsilon);

private:
    int ocl_device_index;
    int algo;

    // optical flow Farn
    int numLevels;
    int numIters;
    int winSize;
    double polySigma;
    double pyrScale;
    int polyN;
    int flags;

    // optical TVL1
    double tau;
    double lambda;
    int warps;
    int nscales;
    int iterations;
    double epsilon;

#if CV_MAJOR_VERSION == 3
    void buildFlowOpenCV_3(cv::UMat& prevgray, cv::UMat& gray, std::string flowfilename);
#else
    void buildFlowOpenCV_CPU(cv::Mat& prevgray, cv::Mat& gray, std::string flowfilename);
#ifdef HAVE_OPENCV_OCL
    void buildFlowOpenCV_OCL(cv::Mat& prevgray, cv::Mat& gray, std::string flowfilename);
    void setupOclDevice();
#endif
#endif

    void dumpAlgosParams();
};

#endif // FLOWSOURCEOPENCV_SV_H
