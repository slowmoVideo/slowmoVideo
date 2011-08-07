#include "flowVisualization_sV.h"

#include <cmath>

#define CLAMP0255(x) ( (x) < 0 ? 0 : (x) > 255 ? 255 : (x) )

QImage FlowVisualization_sV::colourizeFlow(const FlowField_sV *flowField, float boost)
{
    QImage img(flowField->width(), flowField->height(), QImage::Format_RGB32);
    int r,g,b;
    for (int y = 0; y < flowField->height(); y++) {
        for (int x = 0; x < flowField->width(); x++) {
            r = 127 + boost*flowField->x(x,y);
            g = 127 + boost*flowField->y(x,y);
            b = boost*std::sqrt(flowField->x(x,y)*flowField->x(x,y) + flowField->y(x,y)*flowField->y(x,y));
            r = CLAMP0255(r);
            g = CLAMP0255(g);
            b = CLAMP0255(b);
            img.setPixel(x, y, qRgb(r,g,b));
        }
    }
    return img;
}
