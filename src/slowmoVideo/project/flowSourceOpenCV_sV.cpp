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

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
// ?
#include "opencv2/gpu/gpumat.hpp"

#include "opencv2/ocl/ocl.hpp" 

#include <QtCore/QTime>
#include <iostream>
#include <fstream>

#include <QList>
 
using namespace cv;
//using namespace cv::ocl; 
//using namespace cv::gpu;


#if 1
/**
 *  list GPU support for OpenCV
 */
void check_gpu()
{
        qDebug() << "check GPU" ;
        int num_devices = gpu::getCudaEnabledDeviceCount();
        qDebug() << "CUDA support : " << num_devices << "found";
        if (num_devices >= 1) {
                for (int i = 0; i < num_devices; ++i) {
                gpu::printShortCudaDeviceInfo(i);

                gpu::DeviceInfo dev_info(i);
                if (!dev_info.isCompatible()) {
            std::cerr << "GPU module isn't built for GPU #" << i << " ("
                 << dev_info.name() << ", CC " << dev_info.majorVersion()
                 << dev_info.minorVersion() << "\n";
        } /* if */
        } /* for */
    } /* CUDA devices */

        qDebug() << "OpenCL support";
        ocl::PlatformsInfo platforms;
        ocl::getOpenCLPlatforms(platforms);

        for(size_t i=0;i<platforms.size();i++) {
                std::cerr << "plateform : " << platforms[i]->platformName <<  " vendor: " << platforms[i]->platformVendor << "\n";
        }

        ocl::DevicesInfo devInfo;
#if 0
        int res = cv::ocl::getOpenCLDevices(devInfo,ocl::CVCL_DEVICE_TYPE_ALL);
#else
        int res = cv::ocl::getOpenCLDevices(devInfo,ocl::CVCL_DEVICE_TYPE_GPU);
#endif
        if (res != 0) {
        for(size_t i = 0 ; i < devInfo.size() ;i++) {
            std::cerr << "Device : " << i << " " << devInfo[i]->deviceName << " is present" << std::endl;
        }
            

		}
        qDebug() << "end OpenCL support";
}

/**
 *  check if OpenCV as OpenCL support
 *
 *  @return 1 if support
 */
int isOCLsupported()
{
	ocl::PlatformsInfo platforms;
    int res = ocl::getOpenCLPlatforms(platforms);	
    return res;
}

/**
 *  return a list of supported OpenCL devices
 *
 *  @return list of devices
 */
QList<QString> oclFillDevices(void)
{
	  ocl::PlatformsInfo platforms;
      ocl::getOpenCLPlatforms(platforms);

      ocl::DevicesInfo devInfo;
      cv::ocl::getOpenCLDevices(devInfo,ocl::CVCL_DEVICE_TYPE_ALL);
      
      QList<QString> device_list;
      
      for(size_t i = 0 ; i < devInfo.size() ;i++) {
            std::cerr << "Device : " << i << " " << devInfo[i]->deviceName << " is present" << std::endl;
            device_list.insert(i,QString::fromStdString(devInfo[i]->deviceName));
      }
      return device_list;
}

#else
void check_gpu() {
	qDebug() << "no OpenCL support";
}

int isOCLsupported() 
{
	return 0;
}
#endif // OpenCL

FlowSourceOpenCV_sV::FlowSourceOpenCV_sV(Project_sV *project) :
    AbstractFlowSource_sV(project)
{
	// for debugging OpenCL support
    check_gpu();
    use_gpu = 0; // default do not use GPU
    method = 0; // default to Farnback
    createDirectories();
}

void FlowSourceOpenCV_sV::initGPUDevice(int dev)
{
   if (dev == -1) {
	qDebug() << "bad OCL device : " << dev << "for rendering not using it !";
	use_gpu = 0;
   } else {
	qDebug() << "using OCL device : " << dev << "for rendering";
	use_gpu = 1;
	ocl::PlatformsInfo platforms;
    ocl::getOpenCLPlatforms(platforms);
    
    ocl::DevicesInfo devInfo;
    cv::ocl::getOpenCLDevices(devInfo,ocl::CVCL_DEVICE_TYPE_ALL);
      
    ocl::setDevice(devInfo[dev]);
    std::cerr << "Device : " << dev << " is " << devInfo[dev]->deviceName << std::endl;
  }
}
     
void FlowSourceOpenCV_sV::chooseAlgo(int algo) {
	qDebug() << "using Optical Flow Algo : " << algo;
    method = algo;
}
        		
void FlowSourceOpenCV_sV::slotUpdateProjectDir()
{
    m_dirFlowSmall.rmdir(".");
    m_dirFlowOrig.rmdir(".");
    createDirectories();
}

void FlowSourceOpenCV_sV::createDirectories()
{
    m_dirFlowSmall = project()->getDirectory("cache/oFlowSmall");
    m_dirFlowOrig = project()->getDirectory("cache/oFlowOrig");
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
    farn.pyrScale = pyrScale;
    farn.polyN = polyN;
    farn.polySigma = polySigma;
    farn.flags = 0;
    
    farn.numLevels = levels;
    farn.winSize = winsize;
    
    //const int iterations = 8; // 10
    farn.numIters = 8;
}

void FlowSourceOpenCV_sV::setupTVL(double thau,double lambda, double pyrScale, double warp)
{

}

FlowField_sV* FlowSourceOpenCV_sV::buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError)
{
    QString flowFileName(flowPath(leftFrame, rightFrame, frameSize));
    
    /// \todo Check if size is equal
    if (!QFile(flowFileName).exists()) {
        
        QTime time;
        time.start();
        
        Mat prevgray, gray;
        Mat_<Point2f> flow;
        QString prevpath = project()->frameSource()->framePath(leftFrame, frameSize);
        QString path = project()->frameSource()->framePath(rightFrame, frameSize);
        //        namedWindow("flow", 1);
        
		qDebug() << "Building flow for left frame " << leftFrame << " to right frame " << rightFrame << "; Size: " << frameSize;
		
        prevgray = imread(prevpath.toStdString(), 0);
        gray = imread(path.toStdString(), 0);
        
        //cvtColor(l1, prevgray, CV_BGR2GRAY);
        //cvtColor(l2, gray, CV_BGR2GRAY);
        
        
        {
            //const int iterations = 8; // 10
            //done outside setupOpticalFlow(3,15,1.2,0.5,5);
            
            if( prevgray.data ) {
                
                if (use_gpu) {
        			qDebug() << "using GPU OCL version";
        			
        			cv::ocl::oclMat d_flowx, d_flowy;
    				farn(ocl::oclMat(prevgray), ocl::oclMat(gray), d_flowx, d_flowy);
                    
    				cv::Mat flowxy[] = {cv::Mat(d_flowx), cv::Mat(d_flowy)};
    				cv::merge(flowxy, 2, flow);
    				
        		} else {
                    if (method) { // DualTVL1
                        qDebug() << "calcOpticalFlowDual_TVL1";
                        // TODO: put this as instance variable
                        Ptr<DenseOpticalFlow> tvl1 = createOptFlow_DualTVL1();
                        //setupTVL(0.25,0.15, 5, 10);
                        // default are 0.25 0.15 5 5
                        //tlv1_->set("tau", tau_);
                        //tvl1->set("lambda",0.05);
                        //alg_->set("lambda", lambda_);
                        //alg_->set("nscales", nscales_);
                        //alg_->set("warps", warps_);
                        tvl1->calc(prevgray, gray, flow);

                    } else { // _FARN_
                        qDebug() << "calcOpticalFlowFarneback";
                        // TODO: check to use prev flow as initial flow ? (flags)
                        calcOpticalFlowFarneback(
                                                 prevgray, gray,
                                                 //gray, prevgray,  // TBD this seems to match V3D output better but a sign flip could also do that
                                                 flow,
                                                 farn.pyrScale, //0.5,
                                                 farn.numLevels, //3,
                                                 farn.winSize, //15,
                                                 farn.numIters, //3,
                                                 farn.polyN, //5,
                                                 farn.polySigma, //1.2,
                                                 farn.flags //0
                                                 );
                    }
                    
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
