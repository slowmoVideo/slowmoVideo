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
        int res = cv::ocl::getOpenCLDevices(devInfo,ocl::CVCL_DEVICE_TYPE_ALL);
        if (res != 0) {
        for(size_t i = 0 ; i < devInfo.size() ;i++) {
            std::cerr << "Device : " << i << " " << devInfo[i]->deviceName << " is present" << std::endl;
        }

       //ocl::getOpenCLDevices(DevicesInfo& devices, int deviceType = CVCL_DEVICE_TYPE_GPU,
        //        const PlatformInfo* platform = NULL);

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
    
    createDirectories();
}

void FlowSourceOpenCV_sV::initGPUDevice(int dev)
{
	qDebug() << "using OCL device : " << dev << "for rendering";
	use_gpu = 1;
	ocl::PlatformsInfo platforms;
    ocl::getOpenCLPlatforms(platforms);
    
    ocl::DevicesInfo devInfo;
    cv::ocl::getOpenCLDevices(devInfo,ocl::CVCL_DEVICE_TYPE_ALL);
      
    ocl::setDevice(devInfo[dev]);
    std::cerr << "Device : " << dev << " is " << devInfo[dev]->deviceName << std::endl;
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
}

#if 0
// use case of OCL    
    cv::ocl::oclMat d_flowx, d_flowy;
    farn(oclMat(frame0), oclMat(frame1), d_flowx, d_flowy);

    cv::Mat flow;
    if (useInitFlow)
    {
        cv::Mat flowxy[] = {cv::Mat(d_flowx), cv::Mat(d_flowy)};
        cv::merge(flowxy, 2, flow);

        farn.flags |= cv::OPTFLOW_USE_INITIAL_FLOW;
        farn(oclMat(frame0), oclMat(frame1), d_flowx, d_flowy);
    }

    cv::calcOpticalFlowFarneback(
        frame0, frame1, flow, farn.pyrScale, farn.numLevels, farn.winSize,
        farn.numIters, farn.polyN, farn.polySigma, farn.flags);

    std::vector<cv::Mat> flowxy;
    cv::split(flow, flowxy);

}

#endif

FlowField_sV* FlowSourceOpenCV_sV::buildFlow(uint leftFrame, uint rightFrame, FrameSize frameSize) throw(FlowBuildingError)
{
    QString flowFileName(flowPath(leftFrame, rightFrame, frameSize));
    
    /// \todo Check if size is equal
    if (!QFile(flowFileName).exists()) {
        
        QTime time;
        time.start();
        
        Mat prevgray, gray, flow;
        QString prevpath = project()->frameSource()->framePath(leftFrame, frameSize);
        QString path = project()->frameSource()->framePath(rightFrame, frameSize);
        //        namedWindow("flow", 1);
        
		qDebug() << "Building flow for left frame " << leftFrame << " to right frame " << rightFrame << "; Size: " << frameSize;
		
        prevgray = imread(prevpath.toStdString(), 0);
        gray = imread(path.toStdString(), 0);
        
        //cvtColor(l1, prevgray, CV_BGR2GRAY);
        //cvtColor(l2, gray, CV_BGR2GRAY);
        if (use_gpu) {
        	qDebug() << "using GPU OCL version";
        }
        
        {
             const int iterations = 8; // 10
            //done outside setupOpticalFlow(3,15,1.2,0.5,5);
            
            if( prevgray.data ) {
                
                calcOpticalFlowFarneback(
                                         prevgray, gray,
                                         //gray, prevgray,  // TBD this seems to match V3D output better but a sign flip could also do that
                                         flow,
                                         farn.pyrScale, //0.5,
                                         farn.numLevels, //3,
                                         farn.winSize, //15,
                                         iterations, //3,
                                         farn.polyN, //5,
                                         farn.polySigma, //1.2,
                                         farn.flags //0
                                         );
                //cvtColor(prevgray, cflow, CV_GRAY2BGR);
                //drawOptFlowMap(flow, cflow, 16, 1.5, CV_RGB(0, 255, 0));
                drawOptFlowMap(flow, flowFileName.toStdString());
                //imshow("flow", cflow);
                //imwrite(argv[4],cflow);
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
