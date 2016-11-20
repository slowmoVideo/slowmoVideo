/*
This file is part of slowmoVideo.
Copyright (C) 2012  Lucas Walter
              2012  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "flowSourceOpenCV_sV.h"
#include "project_sV.h"
#include "abstractFrameSource_sV.h"
#include "../lib/flowRW_sV.h"
#include "../lib/flowField_sV.h"

#include <QtCore/QTime>
#include <iostream>
#include <fstream>

#include <QList>

using namespace cv;

FlowSourceOpenCV_sV::FlowSourceOpenCV_sV(Project_sV *project, int _algo, int _ocl_dev_idx) :
    AbstractFlowSource_sV(project)
{
    ocl_device_index = _ocl_dev_idx;
    algo = _algo;
    createDirectories();
}

/**
 *  create a optical flow file
 *
 *  @param flow     optical flow to save
 *  @param flowname file name for optical flow
 */
void drawOptFlowMap(const Mat& flow, std::string flowname )
{

  FlowField_sV flowField(flow.cols, flow.rows);

  //qDebug() << "flow is : " << flow.cols << " by " << flow.rows;
  for(int y = 0; y < flow.rows; y++)
        for(int x = 0; x < flow.cols; x++) {
            const Point2f& fxyo = flow.at<Point2f>(y, x);

            flowField.setX(x, y, fxyo.x);
            flowField.setY(x, y, fxyo.y);
        }

  FlowRW_sV::save(flowname, &flowField);
}

void drawOptFlowMapSeparateXandY(const Mat& flowx, const Mat& flowy, std::string flowname )
{
  FlowField_sV flowField(flowx.cols, flowy.rows);
  for(int y = 0; y < flowx.rows; y++) {
      for(int x = 0; x < flowx.cols; x++) {
          const float flowx_float = flowx.at<float>(y, x);
          const float flowy_float = flowy.at<float>(y, x);
            flowField.setX(x, y, flowx_float);
            flowField.setY(x, y, flowy_float);
      }
  }
  FlowRW_sV::save(flowname, &flowField);
}

/**
 *  build path of flow file
 *
 *  @param leftFrame  left frame for flow
 *  @param rightFrame right frame
 *  @param frameSize  resolution (small/orig)
 *
 *  @return name of flow file
 */
const QString FlowSourceOpenCV_sV::flowPath(const uint leftFrame, const uint rightFrame, const FrameSize frameSize) const
{
    QDir dir;
    if (frameSize == FrameSize_Orig) {
        dir = m_dirFlowOrig;
    } else {
        dir = m_dirFlowSmall;
    }
    QString direction;
    if (leftFrame < rightFrame) {
        direction = "forward";
    } else {
        direction = "backward";
    }

    return dir.absoluteFilePath(QString("ocv-%1-%2-%3.sVflow").arg(direction).arg(leftFrame).arg(rightFrame));
}

/**
 *  setup parameter value for flow algorithm
 *
 *  @param levels    number of pyramide level
 *  @param winsize   windows size
 *  @param polySigma sigma
 *  @param pyrScale  pyramide scale
 *  @param polyN     <#polyN description#>
 */
void FlowSourceOpenCV_sV::setupOpticalFlow(const int levels, const int winsize, const double polySigma,
                                           const double pyrScale, const int polyN)
{
    qDebug() << "setup Optical Flow ";
    this->pyrScale = pyrScale;
    this->polyN = polyN;
    this->polySigma = polySigma;
    this->flags = 0;
    this->numLevels = levels;
    this->winSize = winsize;
    //const int iterations = 8; // 10
    this->numIters = 8;

}

void FlowSourceOpenCV_sV::setupTVL1(const double tau, const double lambda, const int nscales, const int warps, const int iterations, const double epsilon)
{
    qDebug() << "setup Optical Flow TLV1";
    this->tau        = tau;
    this->lambda     = lambda;
    this->nscales    = nscales;
    this->warps      = warps;
    this->iterations = iterations;
    this->epsilon    = epsilon;
}

FlowField_sV* FlowSourceOpenCV_sV::buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError)
{
#if CV_MAJOR_VERSION == 2
#ifdef HAVE_OPENCV_OCL
    if (ocl_device_index >= 0) {
        setupOclDevice();
    }
#endif
#endif
    QString flowFileName(flowPath(leftFrame, rightFrame, frameSize));

    /// \todo Check if size is equal
    if (!QFile(flowFileName).exists()) {
        QTime time;
        time.start();
        QString prevpath = project()->frameSource()->framePath(leftFrame, frameSize);
        QString path = project()->frameSource()->framePath(rightFrame, frameSize);

        qDebug() << "Building flow for left frame " << leftFrame << " to right frame " << rightFrame << "; Size: " << frameSize;

        // check if file have been generated !
        //TODO: maybe better error handling ?
        if (!QFile(prevpath).exists())
            throw FlowBuildingError(QString("Could not read image " + prevpath));

        if (!QFile(path).exists())
            throw FlowBuildingError(QString("Could not read image " + path));

        cv::Mat prevgray, gray;
        prevgray = cv::imread(prevpath.toStdString(), CV_LOAD_IMAGE_ANYDEPTH);
        gray = cv::imread(path.toStdString(), CV_LOAD_IMAGE_ANYDEPTH);
#if CV_MAJOR_VERSION == 3
        cv::UMat uprevgray, ugray;
        prevgray.copyTo(uprevgray);
        gray.copyTo(ugray);
#endif

        {
            if (!prevgray.empty()) {
#if CV_MAJOR_VERSION == 3
                buildFlowOpenCV_3(uprevgray, ugray, flowFileName.toStdString());
#else
#ifdef HAVE_OPENCV_OCL
                if (ocl_device_index >= 0) {
                    buildFlowOpenCV_OCL(prevgray, gray, flowFileName.toStdString());
                } else {
                    buildFlowOpenCV_CPU(prevgray, gray, flowFileName.toStdString());
                }
#else
                buildFlowOpenCV_CPU(prevgray, gray, flowFileName.toStdString());
#endif
#endif
            } else {
                qDebug() << "imread: Could not read image " << prevpath;
                throw FlowBuildingError(QString("imread: Could not read image " + prevpath));
            }
        }
        qDebug() << "Optical flow built for " << flowFileName << " in " << time.elapsed() << " ms.";
    } else {
        qDebug().nospace() << "Re-using existing flow image for left frame " << leftFrame << " to right frame " << rightFrame << ": " << flowFileName;
    }

    try {
        return FlowRW_sV::load(flowFileName.toStdString());
    } catch (FlowRW_sV::FlowRWError &err) {
        throw FlowBuildingError(err.message.c_str());
    }
}

void FlowSourceOpenCV_sV::dumpAlgosParams()
{
    if (algo == 1) { // DualTVL1
        qDebug() << "flow via TLV1 algo." << " lambda:" <<
            lambda << " tau:" << tau << " nscales:" << nscales <<
            "warps:" << warps << " iterations:" << iterations <<
            "epsilon:" << epsilon;
    } else { // _FARN_
        qDebug() << "flow via Farneback algo." <<
            " pyrScale:" << pyrScale << " numLevels:" <<
            numLevels << " winSize:" << winSize << " numIters:" <<
            numIters << " polyN:" << polyN << " polySigma:" <<
            polySigma << " flags:" << flags;
    }
}

#if CV_MAJOR_VERSION == 3
void FlowSourceOpenCV_sV::buildFlowOpenCV_3(cv::UMat& uprevgray, cv::UMat& ugray, std::string flowfilename)
{
    dumpAlgosParams();
    qDebug() << "Have OpenCL: " << cv::ocl::haveOpenCL() << " useOpenCL:" << cv::ocl::useOpenCL();
    UMat uflow;
    if (algo == 1) { // DualTVL1
        cv::Ptr<cv::DualTVL1OpticalFlow> tvl1 = cv::createOptFlow_DualTVL1();
        tvl1->setLambda(lambda);
        tvl1->setTau(tau);
        tvl1->setScalesNumber(nscales);
        tvl1->setWarpingsNumber(warps);
        tvl1->setOuterIterations(iterations);
        tvl1->setEpsilon(epsilon);
        tvl1->calc(
                uprevgray,
                ugray,
                uflow
                );
    } else { // _FARN_
        calcOpticalFlowFarneback(
                uprevgray,
                ugray,
                uflow,
                pyrScale, //0.5,
                numLevels, //3,
                winSize, //15,
                numIters, //8,
                polyN, //5,
                polySigma, //1.2,
                flags //0
                );
    }
    Mat flow;
    uflow.copyTo(flow);
    qDebug() << "finished";
    drawOptFlowMap(flow, flowfilename);
}

#else // start CV_MAJOR_VERSION != 3

void FlowSourceOpenCV_sV::buildFlowOpenCV_CPU(cv::Mat& prevgray, cv::Mat& gray, std::string flowfilename)
{
    dumpAlgosParams();
    cv::Mat_<cv::Point2f> flow;
    if (algo == 1) { // DualTVL1
        cv::Ptr<cv::DenseOpticalFlow> tvl1 = cv::createOptFlow_DualTVL1();
        tvl1->set("lambda", lambda);
        tvl1->set("tau", tau);
        tvl1->set("nscales", nscales);
        tvl1->set("warps", warps);
        tvl1->set("iterations", iterations);
        tvl1->set("epsilon", epsilon);
        tvl1->calc(prevgray, gray, flow);
    } else { // _FARN_
        // TODO: check to use prev flow as initial flow ? (flags)
        //gray, prevgray,  // TBD this seems to match V3D output better but a sign flip could also do that
        calcOpticalFlowFarneback(
                prevgray,
                gray,
                flow,
                pyrScale, //0.5,
                numLevels, //3,
                winSize, //15,
                numIters, //8,
                polyN, //5,
                polySigma, //1.2,
                flags //0
                );
    }
    qDebug() << "finished";
    drawOptFlowMap(flow, flowfilename);
}

#ifdef HAVE_OPENCV_OCL
/**
 * OpenCV2 OCL algos have memleaks.
 */
void FlowSourceOpenCV_sV::buildFlowOpenCV_OCL(cv::Mat& prevgray, cv::Mat& gray, std::string flowfilename)
{
    dumpAlgosParams();
    using namespace cv::ocl;
    oclMat ocl_flowx, ocl_flowy;
    if (algo == 1) {
        OpticalFlowDual_TVL1_OCL tvl1_ocl_alg;
        tvl1_ocl_alg.tau        = tau;
        tvl1_ocl_alg.lambda     = lambda;
        tvl1_ocl_alg.nscales    = nscales;
        tvl1_ocl_alg.warps      = warps;
        tvl1_ocl_alg.epsilon    = epsilon;
        tvl1_ocl_alg.iterations = iterations;
        tvl1_ocl_alg(oclMat(prevgray), oclMat(gray), ocl_flowx, ocl_flowy);
        tvl1_ocl_alg.collectGarbage();
    } else {
        FarnebackOpticalFlow farneback_ocl_algo;
        farneback_ocl_algo.numLevels = numLevels;
        farneback_ocl_algo.pyrScale = pyrScale;
        farneback_ocl_algo.pyrScale = pyrScale;
        farneback_ocl_algo.winSize = winSize;
        farneback_ocl_algo.numIters = numIters;
        farneback_ocl_algo.polyN = polyN;
        farneback_ocl_algo.polySigma = polySigma;
        farneback_ocl_algo.flags = flags;
        farneback_ocl_algo(oclMat(prevgray), oclMat(gray), ocl_flowx, ocl_flowy);
        farneback_ocl_algo.releaseMemory();
    }
    Mat flowx, flowy;
    ocl_flowx.download(flowx);
    ocl_flowy.download(flowy);
    drawOptFlowMapSeparateXandY(flowx, flowy, flowfilename);
}

void FlowSourceOpenCV_sV::setupOclDevice() {
    qDebug() << "using olc device index: " << ocl_device_index;
    using namespace cv::ocl;
    PlatformsInfo platform_infos;
    getOpenCLPlatforms(platform_infos);
    int index = 0;
    for (unsigned int i = 0; i < platform_infos.size(); i++) {
        const PlatformInfo *pi = platform_infos[i];
        for (unsigned int j = 0; j < pi->devices.size(); j++) {
            if (index == ocl_device_index) {
                const DeviceInfo *dic = pi->devices[j];
                DeviceInfo *di = (DeviceInfo *)dic;
                di->deviceName = "ocl_devicename_slowmovideo";
                setDevice(di);
                break;
            }
        }
    }
}
#endif // end if HAVE_OPENCV_OCL
#endif // above CV_MAJOR_VERSION == 2
