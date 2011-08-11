#include "flowVisualization_sV.h"

#include <QColor>
#include <cmath>

#define CLAMP0255(x) ( (x) < 0 ? 0 : (x) > 255 ? 255 : (x) )

QImage FlowVisualization_sV::colourizeFlow(const FlowField_sV *flowField, ColourizingType type, float boost)
{
    switch (type) {
    case WXY:
        return colourizeFlowWXY(flowField, boost);
    case HSV:
        return colourizeFlowHSV(flowField, boost);
    default:
        exit(33);
    }
}

QImage FlowVisualization_sV::colourizeFlowWXY(const FlowField_sV *flowField, float boost)
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

QImage FlowVisualization_sV::colourizeFlowHSV(const FlowField_sV *flowField, float boost)
{
    QImage img(flowField->width(), flowField->height(), QImage::Format_RGB32);
    int h, s, v;
    float dx, dy;
    float r, phi;
    for (int y = 0; y < flowField->height(); y++) {
        for (int x = 0; x < flowField->width(); x++) {
            dx = boost*flowField->x(x,y);
            dy = boost*flowField->y(x,y);

//            // Variant a
//            r = std::sqrt(dx*dx + dy*dy);
//            if (r == 0) {
//                phi = 0;
//            } else if (y >= 0) {
//                phi = std::acos(dx/r);
//            } else {
//                phi = -std::acos(dx/r);
//            }
//            // Variant b
//            if (r+x == 0) {
//                phi = M_PI;
//            } else {
//                phi = 2*std::atan(dy/(r+dx));
//            }

            // Variant easiest ...
            phi = std::atan2(dx, dy);

            phi += M_PI;
            h = 359*phi/(2*M_PI);
            s = r;
            v = 255;
            if (s > 255) {
                v = 255 - (r-255);
                s = 255;
                if (v < 0) {
                    v = 0;
                }
            }
            img.setPixel(x, y, QColor::fromHsv(h, s, v).rgb());
        }
    }
    return img;
}


