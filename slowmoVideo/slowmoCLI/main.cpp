/*
slowmoCLI is a command-line interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/


#include <iostream>
#include <cmath>

#include <QString>

#include <QImage>
#include <QColor>
#include <QRgb>

#include <QDebug>

#include "../lib/interpolate_sV.h"


const int RET_MISSING_PARAM = -1;
const int RET_WRONG_PARAM = -2;
const int RET_MISSING_FILE = -3;


char *myName;

enum FlowMode {
    FlowMode_Forward,
    FlowMode_Twoway,
    FlowMode_Undef,
};


void printUsage() {
    std::cout << "Usage: " << std::endl;
    std::cout << "\t" << myName << " twoway <left image> <right image> <flow image> <reverse image> [<output pattern> [numberOffset] ]" << std::endl;
    std::cout << "\t" << myName << " forward <left image> <flow image> [<output pattern> [numberOffset] ]" << std::endl;
}


char* nextArg(int argc, int &argi, char *argv[])
{
    argi++;
    if (argi < argc) {
        std::cout << "Arg: " << argv[argi] << std::endl;
        return argv[argi];
    } else {
        std::cout << "Argument " << argi << " missing." << std::endl;
        printUsage();
        exit(RET_MISSING_PARAM);
    }
}

char* nextOptArg(int argc, int &argi, char *argv[], char *defaultParam)
{
    argi++;
    if (argi < argc) {
        std::cout << "Optional argument: " << argv[argi] << std::endl;
        return argv[argi];
    } else {
        std::cout << "Optional argument " << argi << " not given." << std::endl;
        return defaultParam;
    }
}


int main(int argc, char *argv[])
{
    myName = argv[0];


    int argi = 0;
    char *arg;


    FlowMode mode = FlowMode_Undef;

    arg = nextArg(argc, argi, argv);
    if (strcmp("forward", arg) == 0) {
        mode = FlowMode_Forward;
    } else if (strcmp("twoway", arg) == 0) {
        mode = FlowMode_Twoway;
    }

    if (mode == FlowMode_Undef) {
        printUsage();
        exit(RET_WRONG_PARAM);
    }


    QImage left, right, flow, flowBack, output;
    switch (mode) {
    case FlowMode_Twoway:
        std::cout << "Running two-way flow." << std::endl;
        left = QImage(nextArg(argc, argi, argv));
        right = QImage(nextArg(argc, argi, argv));
        flow = QImage(nextArg(argc, argi, argv));
        flowBack = QImage(nextArg(argc, argi, argv));
        break;
    case FlowMode_Forward:
        std::cout << "Running forward flow." << std::endl;
        left = QImage(nextArg(argc, argi, argv));
        //right = QImage(nextArg(argc, argi, argv));
        flow = QImage(nextArg(argc, argi, argv));
        break;
    case FlowMode_Undef:
        Q_ASSERT(false);
        break;
    }
    QString pattern(nextOptArg(argc, argi, argv, "output%1.png"));
    if (!pattern.contains("%1")) {
        std::cout << "Error: Output pattern must contain a %1 for the image number. Example: output%1.png." << std::endl;
        return RET_WRONG_PARAM;
    }
    bool ok;
    int numberOffset = QString(nextOptArg(argc, argi, argv, "0")).toInt(&ok);
    if (!ok) {
        std::cout << "Error converting argument to number." << std::endl;
        return RET_WRONG_PARAM;
    }

    switch (mode) {
    case FlowMode_Twoway:
        if (flowBack.isNull()) {
            std::cout << "Backward flow image does not exist." << std::endl;
            exit(RET_MISSING_FILE);
        }
        if (flowBack.size() != left.size()) {
            qDebug() << "Re-scaling backward flow image from " << flow.width() << "x" << flow.height() << " to " << left.width() << "x" << left.height();
            flowBack = flowBack.scaled(left.size());
        }
        if (right.isNull()) {
            std::cout << "Right image does not exist." << std::endl;
            exit(RET_MISSING_FILE);
        }
        // Fall through
    case FlowMode_Forward:
        if (left.isNull()) {
            std::cout << "Left image does not exist." << std::endl;
            exit(RET_MISSING_FILE);
        }
        if (flow.isNull()) {
            std::cout << "Flow image does not exist." << std::endl;
            exit(RET_MISSING_FILE);
        }
        if (flow.size() != left.size()) {
            qDebug() << "Re-scaling forward flow image from " << flow.width() << "x" << flow.height() << " to " << left.width() << "x" << left.height();
            flow = flow.scaled(left.size());
        }
        break;
    }



    output = QImage(left.size(), QImage::Format_RGB32);





    const unsigned int steps = 25;
    //const int stepLog = ceil(log10(numberOffset + steps));
    const int stepLog = 8;
    float pos;
    const QChar fillChar = QLatin1Char('0');
    qDebug() << stepLog << ": max length";
    QString filename;

    for (unsigned int step = 0; step < steps+1; step++) {
	pos = step/float(steps);
        if (mode == FlowMode_Twoway) {
            Interpolate_sV::twowayFlow(left, right, flow, flowBack, pos, output);
        } else if (mode == FlowMode_Forward) {
            Interpolate_sV::forwardFlow(left, flow, pos, output);
        }
        filename = pattern.arg(QString::number(numberOffset + step), stepLog, fillChar);
        qDebug() << "Saving position " << pos << " to image " << filename;
        output.save(filename);
    }
}
