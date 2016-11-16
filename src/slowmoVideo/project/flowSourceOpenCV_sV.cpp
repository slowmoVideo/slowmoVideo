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

#include "opencv2/core/version.hpp"


#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <QtCore/QTime>
#include <iostream>
#include <fstream>

#include <QList>
 
using namespace cv;


void check_gpu() {
	qDebug() << "no OpenCL support";
}

int isOCLsupported() 
{
	return 0;
}

QList<QString> oclFillDevices(void)
{
      QList<QString> device_list;
      return device_list;
}

FlowSourceOpenCV_sV::FlowSourceOpenCV_sV(Project_sV *project) :
    AbstractFlowSource_sV(project)
{
    use_gpu = 0; // default do not use GPU
    method = 0; // default to Farnback
    createDirectories();
}

void FlowSourceOpenCV_sV::initGPUDevice(int dev)
{
        qDebug() << "Transparent API OCL device TODO";
}
     
void FlowSourceOpenCV_sV::chooseAlgo(int algo) {
	qDebug() << "using Optical Flow Algo : " << algo;
    method = algo;
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
void FlowSourceOpenCV_sV::setupOpticalFlow(const int levels,const int winsize,const double polySigma,
                                           const double pyrScale,
                                           const int polyN)
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

void FlowSourceOpenCV_sV::setupTVL(double thau,double lambda, double pyrScale, double warp)
{
	qDebug() << "setup Optical Flow TLV";
}

FlowField_sV* FlowSourceOpenCV_sV::buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError)
{
    QString flowFileName(flowPath(leftFrame, rightFrame, frameSize));
    
    /// \todo Check if size is equal
    if (!QFile(flowFileName).exists()) {
        
        QTime time;
        time.start();
        
        cv::Mat prevgray, gray;
        cv::Mat_<cv::Point2f> flow;
        QString prevpath = project()->frameSource()->framePath(leftFrame, frameSize);
        QString path = project()->frameSource()->framePath(rightFrame, frameSize);
        //        namedWindow("flow", 1);
        
        qDebug() << "Building flow for left frame " << leftFrame << " to right frame " << rightFrame << "; Size: " << frameSize;
        
        // any previous flow file ?
        QString prevflowFileName(flowPath(leftFrame-1, rightFrame-1, frameSize));
        if (!QFile(prevflowFileName).exists()) {
            qDebug() << "will try to use " << prevflowFileName << " as initial flow";
            // load flow file ,
        }
        
        // check if file have been generated !
        //TODO: maybe better error handling ?
        if (!QFile(prevpath).exists())
            throw FlowBuildingError(QString("Could not read image " + prevpath));
        
        if (!QFile(path).exists())
            throw FlowBuildingError(QString("Could not read image " + path));
        
        prevgray = cv::imread(prevpath.toStdString(), CV_LOAD_IMAGE_ANYDEPTH);
        gray = cv::imread(path.toStdString(), CV_LOAD_IMAGE_ANYDEPTH);
        
        //cvtColor(l1, prevgray, CV_BGR2GRAY);
        //cvtColor(l2, gray, CV_BGR2GRAY);
        
        
        {
            //const int iterations = 8; // 10
            //done outside setupOpticalFlow(3,15,1.2,0.5,5);
            
            if( prevgray.data ) {
                
                if (method) { // DualTVL1
                    qDebug() << "calcOpticalFlowDual_TVL1";
#if CV_MAJOR_VERSION == 3
                    // TODO: put this as instance variable
                    cv::Ptr<cv::DualTVL1OpticalFlow> tvl1 = cv::createOptFlow_DualTVL1();
                    //setupTVL(0.25,0.15, 5, 10);
                    // default are 0.25 0.15 5 5
									  tvl1->setLambda(0.05);
                    //tlv1_->set("tau", tau_);
                    //tvl1->set("lambda",0.05);
                    //qDebug() <<  "lambda : " <<  tvl1->getLambda();
                    //alg_->set("lambda", lambda_);
                    //alg_->set("nscales", nscales_);
                    //alg_->set("warps", warps_);
                    tvl1->calc(prevgray, gray, flow);
#else
                    qDebug() << "calcOpticalFlowDual_TVL1 not supported";
#endif 
                } else { // _FARN_
                    qDebug() << "calcOpticalFlowFarneback";
                    // TODO: check to use prev flow as initial flow ? (flags)
                    calcOpticalFlowFarneback(
                                             prevgray, gray,
                                             //gray, prevgray,  // TBD this seems to match V3D output better but a sign flip could also do that
                                             flow,
                                             pyrScale, //0.5,
                                             numLevels, //3,
                                             winSize, //15,
                                             numIters, //3,
                                             polyN, //5,
                                             polySigma, //1.2,
                                             flags //0
                                             );
                }
                
                drawOptFlowMap(flow, flowFileName.toStdString());
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



/*
 * prebuilt the flow files
 */
void FlowSourceOpenCV_sV::buildFlowForwardCache(FrameSize frameSize) throw(FlowBuildingError)
{
    int lastFrame = project()->frameSource()->framesCount();
    int frame = 0;
    Mat prevgray, gray, flow;
    
    qDebug() << "Pre Building forward flow for Size: " << frameSize;
    
    // load first frame
    QString prevpath = project()->frameSource()->framePath(frame, frameSize);
    prevgray = imread(prevpath.toStdString(), 0);
    
    // TODO: need sliders for all these parameters
    const int levels = 3; // 5
    const int winsize = 15; // 13
    const int iterations = 8; // 10
    
    const double polySigma = 1.2;
    const double pyrScale = 0.5;
    const int polyN = 5;
    const int flags = 0;
    
    for(frame=0;frame<lastFrame;frame++) {
        QString flowFileName(flowPath(frame, frame+1, frameSize));
        
        qDebug() << "Building flow for left frame " << frame << " to right frame " << frame+1 << "; Size: " << frameSize;
        /// \todo Check if size is equal
        if (!QFile(flowFileName).exists()) {
            
            //QTime time;
            //time.start();
            
            QString prevpath = project()->frameSource()->framePath(frame, frameSize);
            QString path = project()->frameSource()->framePath(frame+1, frameSize);
            
            gray = imread(path.toStdString(), 0);
            
            // use previous flow info
            //if (frame!=0)
            //    flags |= OPTFLOW_USE_INITIAL_FLOW;
            
            calcOpticalFlowFarneback(
                                     prevgray, gray,
                                     flow,
                                     pyrScale, //0.5,
                                     levels, //3,
                                     winsize, //15,
                                     iterations, //3,
                                     polyN, //5,
                                     polySigma, //1.2,
                                     flags //0 OPTFLOW_USE_INITIAL_FLOW
                                     );
            // save result
            drawOptFlowMap(flow, flowFileName.toStdString());
            std::swap(prevgray, gray);
            qDebug() << "Optical flow built for " << flowFileName;
            
        } else {
            qDebug().nospace() << "Re-using existing flow image for left frame " << frame << " to right frame " << frame+1 << ": " << flowFileName;
        }
        //qDebug() << "Optical flow built for " << flowFileName << " in " << time.elapsed() << " ms.";
        
    }
    
}
