
#include <iostream>

#include <QImage>
#include <QtCore/QDebug>

#include "../lib/flowRW_sV.h"

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

    int width;
    int height;
    float *flowData = FlowRW_sV::load(inputFile, width, height);

    std::cout << "Flow file loaded. Width: " << width << ", height: " << height << std::endl;

    QImage img(width, height, QImage::Format_RGB32);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            img.setPixel(x, y, qRgb(
                             (int) (127 + flowData[2*(y*width + x)+0]),
                             (int) (127 + flowData[2*(y*width + x)+1])
                             , 0)
                         );
        }
    }

    img.save(outputFile);

}
