/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/



#include "../lib/flowRW_sV.h"
#include "../lib/flowField_sV.h"

#include <QImage>
#include <QtCore/QDebug>

#include <iostream>
#include <math.h>

char *myName;

void printUsage() {
    std::cout << "Usage: " << std::endl;
    std::cout << "\t" << myName << " <flow data> <output image>" << std::endl;
}

int main(int argc, char *argv[])
{
    myName = argv[0];

    if (argc <= 2) {
        printUsage();
        exit(-1);
    }

    std::string inputFile = argv[1];
    QString outputFile(argv[2]);

    FlowField_sV *flowField = FlowRW_sV::load(inputFile);

    std::cout << "Flow file loaded. Width: " << flowField->width() << ", height: " << flowField->height() << std::endl;

    QImage img(flowField->width(), flowField->height(), QImage::Format_RGB32);
    for (int y = 0; y < flowField->height(); y++) {
        for (int x = 0; x < flowField->width(); x++) {
            img.setPixel(x, y, qRgb(
                             (int) (127 + flowField->x(x,y)),
                             (int) (127 + flowField->y(x,y)),
                             (int) sqrt(flowField->x(x,y)*flowField->x(x,y) + flowField->y(x,y)*flowField->y(x,y))
                         ));
        }
    }

    std::cout << "Saving " << outputFile.toStdString() << " ..." << std::endl;
    img.save(outputFile);

    delete flowField;
}
