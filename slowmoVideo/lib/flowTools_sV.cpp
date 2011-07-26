#include "flowTools_sV.h"
#include <cmath>

void FlowTools_sV::difference(const FlowField_sV &left, const FlowField_sV &right, FlowField_sV &out)
{
    float dx, dy;
    for (int y = 0; y < left.height(); y++) {
        for (int x = 0; x < left.width(); x++) {
            dx = left.x(x,y);
            dy = left.y(x,y);
            if (x+dx >= 0 && y+dy >= 0
                    && x+dx <= left.width()-1 && y+dy <= left.height()-1) {
                dx += right.x(x+dx, y+dy);
                dy += right.y(x+dx, y+dy);
            }
            out.setX(x,y, dx);
            out.setY(x,y, dy);
        }
    }
}
void FlowTools_sV::signedDifference(const FlowField_sV &left, const FlowField_sV &right, FlowField_sV &out)
{
    float lx, ly;
    float rx, ry;
    for (int y = 0; y < left.height(); y++) {
        for (int x = 0; x < left.width(); x++) {
            lx = left.x(x,y);
            ly = left.y(x,y);
            if (x+lx >= 0 && y+ly >= 0
                    && x+lx <= left.width()-1 && y+ly <= left.height()-1) {
                rx = right.x(x+lx, y+ly);
                ry = right.y(x+lx, y+ly);
                if (fabs(lx)+fabs(ly) > fabs(rx)+fabs(ry)) {
                    lx = fabs(lx+rx);
                    ly = fabs(ly+ry);
                } else {
                    lx = -fabs(lx+rx);
                    ly = -fabs(ly+ry);
                }
            } else {
                lx = fabs(lx);
                rx = fabs(rx);
            }
            out.setX(x,y, lx);
            out.setY(x,y, ly);
        }
    }
}
