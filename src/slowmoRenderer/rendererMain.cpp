/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "slowmoRenderer_sV.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

#include <iostream>
#include <csignal>
#include <cstdlib>

QString myName;
SlowmoRenderer_sV renderer;

int terminateCounter = 0;
int genproj = 0;

//TODO: maybe in case of abort we should remove directories ?
void terminate(int)
{
    if (terminateCounter == 0) {
        std::cout << "Telling renderer to stop." << std::endl;
    } else if (terminateCounter == 1) {
        std::cout << "Really want to kill rendering? Send the SIGINT a third time." << std::endl;
    } else {
        exit(SIGINT);
    }
    terminateCounter++;
    renderer.abort();
}

void printProgress(int)
{
    renderer.printProgress();
}

void printHelp()
{
    std::cout << "slowmoRenderer for slowmoVideo " << Version_sV::version.toStdString() << std::endl
              << myName.toStdString() << " [<project>]" << std::endl
              << "\t-target [video <path> [<codec>|auto] | images <filenamePattern> <directory> ] " << std::endl
              << "\t-input video <path> | images <filenamePattern> <directory> ] " << std::endl
              << "\t-size [small|orig] " << std::endl
              << "\t-fps <fps> " << std::endl
              << "\t-start <startTime> -end <endTime> " << std::endl
              << "\t-interpolation [forward[2]|twoway[2]] " << std::endl
              << "\t -motionblur [stack|convolve] " << std::endl
              << "\t -slowfactor <factor>" << std::endl
              << "\t-v3dLambda <lambda> " << std::endl;
}

void require(int nArgs, int index, int size)
{
    if (size <= index + nArgs) {
        std::cout << "Not enough arguments delivered (" << nArgs << " required)." << std::endl;
        printHelp();
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Set up preferences for the QSettings file
    QCoreApplication::setOrganizationName("Granjow");
    QCoreApplication::setOrganizationDomain("granjow.net");
    QCoreApplication::setApplicationName("slowmoUI");

    if (signal(SIGINT, terminate) == SIG_ERR) {
        std::cerr << "Could not set up SIGINT handler." << std::endl;
    }
#ifndef WINDOWS
    if (signal(SIGUSR1, printProgress) == SIG_ERR) {
        std::cerr << "Could not set up SIGUSR1 handler." << std::endl;
    }
#endif

    QStringList args = app.arguments();
    myName = args.at(0);
    if (argc <= 1
            || "--help" == args.at(1) || "-h" == args.at(1)) {
        printHelp();
        return 0;
    }



    int next = 1;
    if ((args.at(1)).contains("svProj", Qt::CaseInsensitive) ) {
	    renderer.load(args.at(1));
	    next = 2;
    } else 
	    renderer.create();

    QString start = ":start";
    QString end = ":end";

    const int n = args.size();
    while (next < n) {
        if ("-target" == args.at(next)) {
            require(3, next, n);
            next++;
            if ("video" == args.at(next)) {
                next++;
                QString filename = args.at(next++);
                QString codec = args.at(next++);
                if ("auto" == codec) { codec = ""; }

                renderer.setVideoRenderTarget(filename, codec);

            } else if ("images" == args.at(next)) {
                next++;
                QString filenamePattern = args.at(next++);
                QString dir = args.at(next++);
                renderer.setImagesRenderTarget(filenamePattern, dir);

            } else {
                std::cerr << "Not a valid target: " << args.at(next).toStdString() << std::endl;
                return -1;
            }

        } else if ("-input" == args.at(next)) {
            require(2, next, n);
            next++;
            if ("video" == args.at(next)) {
                next++;
                QString filename = args.at(next++);
                
                renderer.setInputTarget(filename);
                
            } else if ("images" == args.at(next)) {
                next++;
                QString filenamePattern = args.at(next++);
                QString dir = args.at(next++);
                //TODO: pattern ?
                //renderer.setImagesRenderTarget(filenamePattern, dir);
                
            } else {
                std::cerr << "Not a valid input: " << args.at(next).toStdString() << std::endl;
                return -1;
            }
        } else if ("-size" == args.at(next)) {
            require(1, next, n);
            next++;
            if ("small" == args.at(next)) {
                renderer.setSize(false);
            } else if ("orig" == args.at(next)) {
                renderer.setSize(true);
            } else {
                std::cerr << "Not a valid size: " << args.at(next).toStdString() << std::endl;
                return -1;
            }
            next++;

        } else if ("-fps" == args.at(next)) {
            require(1, next, n);
            next++;
            bool b;
            double fps = args.at(next).toDouble(&b);
            if (!b) {
                std::cerr << "Not a number: " << args.at(next).toStdString() << std::endl;
                return -1;
            }
            renderer.setFps(fps);
            next++;

        } else if ("-start" == args.at(next)) {
            require(1, next, n);
            next++;
            start = args.at(next++);

        } else if ("-end" == args.at(next)) {
            require(1, next, n);
            next++;
            end = args.at(next++);

        } else if ("-interpolation" == args.at(next)) {
            require(1, next, n);
            next++;
            if ("forward" == args.at(next)) {
                renderer.setInterpolation(InterpolationType_Forward);
            } else if ("forward2" == args.at(next)) {
                renderer.setInterpolation(InterpolationType_ForwardNew);
            } else if ("twoway" == args.at(next)) {
                renderer.setInterpolation(InterpolationType_Twoway);
            } else if ("twoway2" == args.at(next)) {
                renderer.setInterpolation(InterpolationType_TwowayNew);
            } else {
                std::cerr << "Not a valid interpolation type: " << args.at(next).toStdString() << std::endl;
                return -1;
            }
            next++;

        } else if ("-motionblur" == args.at(next)) {
            require(1, next, n);
            next++;
            if ("stack" == args.at(next)) {
                renderer.setMotionblur(MotionblurType_Stacking);
            } else if ("convolve" == args.at(next)) {
                renderer.setMotionblur(MotionblurType_Convolving);
            } else {
                std::cerr << "Not a valid motion blur type: " << args.at(next).toStdString() << std::endl;
                return -1;
            }
            next++;

        } else if ("-v3dLambda" == args.at(next)) {
            require(1, next, n);
            next++;
            bool b;
            float lambda = args.at(next).toFloat(&b);
            if (!b) {
                std::cerr << "Not a number: " << args.at(next).toStdString() << std::endl;
                return -1;
            }
            renderer.setV3dLambda(lambda);
            next++;

        } else if ("-slowfactor" == args.at(next)) {
            require(1, next, n);
            next++;
            bool b;
            double slowfactor = args.at(next).toDouble(&b);
            if (!b) {
                std::cerr << "Not a number: " << args.at(next).toStdString() << std::endl;
                return -1;
            }
            std::cerr << "will slow down to : " << slowfactor << std::endl;
	    renderer.setSpeed(slowfactor);
            next++;
	} else {
            std::cout << "Argument not recognized: " << args.at(next).toStdString() << std::endl;
            printHelp();
            return -1;
        }
    }

    renderer.setTimeRange(start, end);

    QString msg;
    if (!renderer.isComplete(msg)) {
        std::cout << msg.toStdString() << std::endl;
        std::cout << "Project will not be rendered." << std::endl;
        return 42;
    }

    if (genproj) 
	renderer.save("test.svProj");
    else
	renderer.start();

}
