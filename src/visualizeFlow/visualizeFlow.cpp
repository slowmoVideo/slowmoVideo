/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/


#include "flowRW_sV.h"
#include "lib/flowVisualization_sV.h"
#include "lib/flowTools_sV.h"

#include <QImage>
#include <QtCore/QDebug>

#include <iostream>

char *myName;

void printUsage() {
    std::cout << "Usage: " << std::endl;
    std::cout << "\t" << myName << " <flow data> <output image>" << std::endl;
    std::cout << "\t" << myName << " diff <flow1> <flow2> <output image>" << std::endl;
    std::cout << "\t" << myName << " ref (writes an HSV reference image)" << std::endl;
}

QImage reference() {
    const int width = 1024, height = 1024;
    float cX = width/2.0;
    float cY = height/2.0;

    FlowField_sV field(width, height);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            field.setX(x,y, x-cX);
            field.setY(x,y, y-cY);
        }
    }

    return FlowVisualization_sV::colourizeFlow(&field, FlowVisualization_sV::HSV, 1.0);
}

void colourizeFlow(int argc, char *argv[])
{
    if (argc <= 2) {
        printUsage();
        exit(-1);
    }
    std::string inputFile = argv[1];
    QString outputFile(argv[2]);

    FlowField_sV *flowField;
    try {
        flowField = FlowRW_sV::load(inputFile);
    } catch (FlowRW_sV::FlowRWError &err) {
        std::cout << err.message << std::endl;
        exit(-2);
    }

    std::cout << "Flow file loaded. Width: " << flowField->width() << ", height: " << flowField->height() << std::endl;

    /// \todo make visualization type configurable
    QImage img = FlowVisualization_sV::colourizeFlow(flowField, FlowVisualization_sV::WXY);

    std::cout << "Saving " << outputFile.toStdString() << " ..." << std::endl;
    img.save(outputFile);

    delete flowField;
}

void diffFlow(int argc, char *argv[])
{
    if (argc <= 4) {
        printUsage();
        exit(-1);
    }
    FlowField_sV *leftFlow = FlowRW_sV::load(argv[2]);
    FlowField_sV *rightFlow = FlowRW_sV::load(argv[3]);
    FlowField_sV flowDifference(leftFlow->width(), leftFlow->height());

    if (strcmp("diffSigned", argv[1]) == 0) {
        FlowTools_sV::signedDifference(*leftFlow, *rightFlow, flowDifference);
    } else {
        FlowTools_sV::difference(*leftFlow, *rightFlow, flowDifference);
    }

    int d;
    QImage img(flowDifference.width(), flowDifference.height(), QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            d = 128 + flowDifference.x(x,y)+flowDifference.y(x,y);
            if (d > 255) { d = 255; }
            if (d < 0) { d = 0; }
            img.setPixel(x, y, qRgb(d,d,d));
        }
    }
    img.save(argv[4]);

    delete leftFlow;
    delete rightFlow;
}

int main(int argc, char *argv[])
{
    myName = argv[0];

    if (argc <= 1) {
        printUsage();
        exit(-1);
    }

    if (strcmp("diff", argv[1]) == 0 || strcmp("diffSigned", argv[1]) == 0) {
        diffFlow(argc, argv);
    } else if (strcmp("ref", argv[1]) == 0) {
        reference().save("reference.png");
    } else {
        colourizeFlow(argc, argv);
    }
}
