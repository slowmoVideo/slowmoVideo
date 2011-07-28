#include "flowVisualization_sV.h"

#include <cmath>

QImage FlowVisualization_sV::colourizeFlow(const FlowField_sV *flowField)
{
    QImage img(flowField->width(), flowField->height(), QImage::Format_RGB32);
    for (int y = 0; y < flowField->height(); y++) {
        for (int x = 0; x < flowField->width(); x++) {
            img.setPixel(x, y, qRgb(
                             (int) (127 + flowField->x(x,y)),
                             (int) (127 + flowField->y(x,y)),
                             (int) std::sqrt(flowField->x(x,y)*flowField->x(x,y) + flowField->y(x,y)*flowField->y(x,y))
                         ));
        }
    }
    return img;
}
