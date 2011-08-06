#include "flowTools_sV.h"
#include <cmath>
#include <cassert>

//#define DEBUG

void FlowTools_sV::refillLine(FlowField_sV &field, int startTop, int startLeft, int length, LineFillMode fillMode)
{
    int x = startLeft;
    int y = startTop;

    float sumX;
    float sumY;
    short count = 0;
    if (fillMode == HorizontalFromTop || fillMode == HorizontalFromBottom || fillMode == HorizontalFromBoth) {
        x++;
        while (x < startLeft+length) {
            sumX = 0;
            sumY = 0;
            count = 0;
            if (fillMode == HorizontalFromTop || fillMode == HorizontalFromBoth) {
                sumX += field.x(x-1, y-1) + field.x(x, y-1) + field.x(x+1, y-1);
                sumY += field.y(x-1, y-1) + field.y(x, y-1) + field.y(x+1, y-1);
                count += 3;
            }
            if (fillMode == HorizontalFromBottom || fillMode == HorizontalFromBoth) {
                sumX += field.x(x-1, y+1) + field.x(x, y+1) + field.x(x+1, y+1);
                sumY += field.y(x-1, y+1) + field.y(x, y+1) + field.y(x+1, y+1);
                count += 3;
            }
#ifdef DEBUG
            sumX = 100;
            sumY = 100;
#endif
            field.rx(x,y) = sumX/count;
            field.ry(x,y) = sumY/count;
            x++;
        }
    } else {
        y++;
        while (y < startTop + length) {
            sumX = 0;
            sumY = 0;
            count = 0;
            if (fillMode == VerticalFromLeft || fillMode == VerticalFromBoth) {
                sumX += field.x(x-1, y-1) + field.x(x-1, y) + field.x(x+1, y);
                sumY += field.y(x-1, y-1) + field.y(x-1, y) + field.y(x+1, y);
                count += 3;
            }
            if (fillMode == VerticalFromRight || fillMode == VerticalFromBoth) {
                sumX += field.x(x+1, y-1) + field.x(x+1, y) + field.x(x+1, y+1);
                sumY += field.y(x+1, y-1) + field.y(x+1, y) + field.y(x+1, y+1);
                count += 3;
            }
#ifdef DEBUG
            sumX = 100;
            sumY = 100;
#endif
            field.rx(x,y) = sumX/count;
            field.ry(x,y) = sumY/count;
            y++;
        }
    }
}

void FlowTools_sV::refillCorner(FlowField_sV &field, int top, int left, CornerPosition pos)
{
    int dx;
    int dy;
    if (pos == TopRight || pos == BottomRight) {
        dx = 1;
    } else {
        dx = -1;
    }
    if (pos == TopRight || pos == TopLeft) {
        dy = -1;
    } else {
        dy = 1;
    }
    float sumX = 0;
    float sumY = 0;
    int count = 0;
    if (left+dx > 0 && top+dy > 0
            && left+dx < field.width() && top+dy < field.height()) {
        sumX += field.x(left+dx, top+dy);
        sumY += field.y(left+dx, top+dy);
        count++;
    }
    if (left+dx > 0 && top-dy > 0
            && left+dx < field.width() && top-dy < field.height()) {
        sumX += field.x(left+dx, top-dy);
        sumY += field.y(left+dx, top-dy);
        count++;
    }
    if (left-dx > 0 && top+dy > 0
            && left-dx < field.width() && top+dy < field.height()) {
        sumX += field.x(left-dx, top+dy);
        sumY += field.y(left-dx, top+dy);
        count++;
    }
#ifdef DEBUG
            sumX = 100;
            sumY = 100;
#endif
    field.rx(left,top) = sumX/count;
    field.ry(left,top) = sumY/count;
}

void FlowTools_sV::refill(FlowField_sV &field, int top, int left, int bottom, int right)
{
    assert(top <= bottom);
    assert(left <= right);
    assert(top >= 0);
    assert(left >= 0);
    assert(bottom < field.height());
    assert(right < field.width());

    // Top line
    if (top > 0){
        if (bottom > top) {
            // Fill from above
            refillLine(field, top, left, right-left+1, HorizontalFromTop);
        } else {
            // Only a line left; fill from both sides
            refillLine(field, top, left, right-left+1, HorizontalFromBoth);
        }
    }
    // Left line
    if (left > 0) {
        if (right > left) {
            refillLine(field, top, left, bottom-top+1, VerticalFromLeft);
        } else {
            refillLine(field, top, left, bottom-top+1, VerticalFromBoth);
        }
    }
    // Right line
    if (right+1 < field.width()) {
        if (left < right) {
            refillLine(field, top, right, bottom-top+1, VerticalFromRight);
        } else {
            refillLine(field, top, right, bottom-top+1, VerticalFromBoth);
        }
    }
    // Bottom line
    if (bottom+1 < field.height()) {
        if (top < bottom) {
            refillLine(field, bottom, left, right-left+1, HorizontalFromBottom);
        } else {
            refillLine(field, bottom, left, right-left+1, HorizontalFromBoth);
        }
    }
    refillCorner(field, top, left, TopLeft);
    refillCorner(field, top, right, TopRight);
    refillCorner(field, bottom, left, BottomLeft);
    refillCorner(field, bottom, right, BottomRight);

    if (bottom-top >= 2 && right-left >= 2) {
        refill(field, top+(top > 0 ? 1 : 0), left+(left > 0 ? 1 : 0),
               bottom-(bottom < field.height()-1 ? 1 : 0), right-(right < field.width()-1 ? 1 : 0));
    }

//    int x = left;
//    int y = top;


//    if (bottom == top && right == left) {

//        field.rx(x,y) = (field.x(x-1, y-1) + field.x(x+1, y-1) + field.x(x+1, y+1) + field.x(x-1, y+1))/4;
//        field.ry(x,y) = (field.y(x-1, y-1) + field.y(x+1, y-1) + field.y(x+1, y+1) + field.y(x-1, y+1))/4;

//    } else if (bottom == top) {

//        while (x <= right) {
//            field.rx(x,y) = (field.x(x, y+1) + field.x(x, y-1))/2;
//            x++;
//        }

//    } else if (right == left) {

//        while (y <= right) {
//            field.ry(x,y) = (field.y(x, y+1) + field.y(x, y-1))/2;
//            y++;
//        }

//    } else {

//        field.rx(x, y) = (field.x(x-1, y+1) + field.x(x-1, y-1) + field.x(x+1, y-1))/3;
//        field.ry(x, y) = (field.y(x-1, y+1) + field.y(x-1, y-1) + field.y(x+1, y-1))/3;
//        x++;
//        while (x < right) {
//            field.rx(x, y) = (field.x(x-1, y-1) + field.x(x, y-1) + field.x(x+1, y-1))/3;
//            field.ry(x, y) = (field.y(x-1, y-1) + field.y(x, y-1) + field.y(x+1, y-1))/3;
//            x++;
//        }
//        field.rx(x,y) = (field.x(x-1, y-1) + field.x(x+1, y-1) + field.x(x+1, y+1))/3;
//        field.ry(x,y) = (field.y(x-1, y-1) + field.y(x+1, y-1) + field.y(x+1, y+1))/3;
//        y++;
//        while (y < bottom) {
//            field.rx(x,y) = (field.x(x+1, y-1) + field.x(x+1, y) + field.x(x+1, y+1))/3;
//            field.ry(x,y) = (field.y(x+1, y-1) + field.y(x+1, y) + field.y(x+1, y+1))/3;
//            y++;
//        }
//        field.rx(x,y) = (field.x(x+1, y-1) + field.x(x+1, y+1) + field.x(x-1, y+1))/3;
//        field.ry(x,y) = (field.y(x+1, y-1) + field.y(x+1, y+1) + field.y(x-1, y+1))/3;
//        x--;
//        while (x > left) {
//            field.rx(x,y) = (field.x(x-1, y+1) + field.x(x, y+1) + field.x(x+1, y+1))/3;
//            field.ry(x,y) = (field.y(x-1, y+1) + field.y(x, y+1) + field.y(x+1, y+1))/3;
//            x--;
//        }
//        field.rx(x,y) = (field.x(x+1, y+1) + field.x(x-1, y+1) + field.x(x-1, y-1))/3;
//        field.ry(x,y) = (field.y(x+1, y+1) + field.y(x-1, y+1) + field.y(x-1, y-1))/3;
//        y--;
//        while (y > top) {
//            field.rx(x,y) = (field.x(x-1, y+1) + field.x(x-1, y) + field.x(x-1, y-1))/3;
//            field.ry(x,y) = (field.y(x-1, y+1) + field.y(x-1, y) + field.y(x-1, y-1))/3;
//            y--;
//        }

//        if (bottom-top >= 2 && right-left >= 2) {
//            refill(field, top+1, left+1, bottom-1, right-1);
//        }

//    }
}

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
