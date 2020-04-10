/*
slowmoCLI is a command-line interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "../lib/interpolate_sV.h"
#include "flowField_sV.h"
#include "flowRW_sV.h"

#include <iostream>
#include <cmath>

#include <QString>

#include <QImage>
#include <QColor>
#include <QRgb>

#include <QDebug>



const int RET_MISSING_PARAM = -1;
const int RET_WRONG_PARAM = -2;
const int RET_MISSING_FILE = -3;
const int RET_SIZE_DIFFERS = -4;


char *myName;

enum FlowMode {
    FlowMode_Forward,
    FlowMode_Twoway,
    FlowMode_Undef
};


void printUsage() {
    std::cout << "Usage: " << std::endl;
    std::cout << "\t" << myName << " twoway <left image> <right image> <flow image> <reverse image> [<output pattern> [numberOffset [fps]] ]" << std::endl;
    std::cout << "\t" << myName << " forward <left image> <flow image> [<output pattern> [numberOffset [fps]] ]" << std::endl;
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

const char* nextOptArg(int argc, int &argi, char *argv[], const char defaultParam[])
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


    QImage left, right, output;
    FlowField_sV *ffForward, *ffBackward;

    switch (mode) {
    case FlowMode_Twoway:
        std::cout << "Running two-way flow." << std::endl;
        left = QImage(nextArg(argc, argi, argv));
        right = QImage(nextArg(argc, argi, argv));
        ffForward = FlowRW_sV::load(nextArg(argc, argi, argv));
        ffBackward = FlowRW_sV::load(nextArg(argc, argi, argv));
        break;
    case FlowMode_Forward:
        std::cout << "Running forward flow." << std::endl;
        left = QImage(nextArg(argc, argi, argv));
        ffForward = FlowRW_sV::load(nextArg(argc, argi, argv));
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
    const unsigned int fps = QString(nextOptArg(argc, argi, argv, "24")).toInt(&ok);
    if (!ok) {
        std::cout << "Error converting argument to number." << std::endl;
        return RET_WRONG_PARAM;
    }



    switch (mode) {
    case FlowMode_Twoway:
        if (ffBackward == NULL) {
            std::cout << "Backward flow is not valid." << std::endl;
            exit(RET_MISSING_FILE);
        }
        if (right.isNull()) {
            std::cout << "Right image does not exist." << std::endl;
            exit(RET_MISSING_FILE);
        }
        if (ffBackward->width() != left.width() || ffBackward->height() != left.height()) {
            qDebug() << "Invalid backward flow field size. Image is " << left.width()
                     << ", flow is " << ffBackward->width() << "x" << ffBackward->height() << ".";
            exit(RET_SIZE_DIFFERS);
        }
        // Fall through
    case FlowMode_Forward:
        if (left.isNull()) {
            std::cout << "Left image does not exist." << std::endl;
            exit(RET_MISSING_FILE);
        }
        if (ffForward == NULL) {
            std::cout << "Forward flow is not valid." << std::endl;
            exit(RET_MISSING_FILE);
        }
        if (left.size() != right.size()) {
            qDebug() << "Left image size differs from right image size: " << left.size() << " vs. " << right.size() << ".";
        }
        if (ffForward->width() != left.width() || ffForward->height() != left.height()) {
            qDebug() << "Invalid forward flow field size. Image is " << left.width()
                     << ", flow is " << ffForward->width() << "x" << ffForward->height() << ".";
            exit(RET_SIZE_DIFFERS);
        }
        break;
    case FlowMode_Undef:
        qDebug() << "Undefined flow mode selected.";
        break;
    }



    output = QImage(left.size(), QImage::Format_RGB32);





    //const int stepLog = ceil(log10(numberOffset + steps));
    const int stepLog = 8;
    float pos;
    const QChar fillChar = QLatin1Char('0');
    qDebug() << stepLog << ": max length";
    QString filename;

    for (unsigned int step = 0; step < fps+1; step++) {
        pos = step/float(fps);
        if (mode == FlowMode_Twoway) {
            Interpolate_sV::twowayFlow(left, right, ffForward, ffBackward, pos, output);
        } else if (mode == FlowMode_Forward) {
            Interpolate_sV::forwardFlow(left, ffForward, pos, output);
        }
        filename = pattern.arg(QString::number(numberOffset + step), stepLog, fillChar);
        qDebug() << "Saving position " << pos << " to image " << filename;
        output.save(filename);
    }

    delete ffForward;
    if (mode == FlowMode_Twoway) {
        delete ffBackward;
    }
}
