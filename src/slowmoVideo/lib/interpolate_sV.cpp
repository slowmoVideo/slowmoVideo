/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "interpolate_sV.h"
#include "flowField_sV.h"
#include "flowTools_sV.h"
#include "sourceField_sV.h"
#include "vector_sV.h"
#include "bezierTools_sV.h"

#ifdef WINDOWS
#include <math.h>
#else
#include <cmath>
#endif

#include <QDebug>
#include <QImage>
#include <QColor>

#define CLAMP1(x) ( ((x) > 1.0) ? 1.0 : (x) )
#define CLAMP(x,min,max) (  ((x) < (min)) ? (min) : ( ((x) > (max)) ? (max) : (x) )  )

#define INTERPOLATE
//#define FIX_FLOW
#define FIX_BORDERS
//#define DEBUG_I

enum ColorComponent { CC_Red, CC_Green, CC_Blue };


inline
float interpR(const QColor cols[2][2], float x, float y)
{
    return (1-x)*(1-y) * cols[0][0].redF()
        + x*(1-y) * cols[1][0].redF()
        + y*(1-x) * cols[0][1].redF()
        + x*y * cols[1][1].redF();
}

inline
float interpG(const QColor cols[2][2], float x, float y)
{
    return (1-x)*(1-y) * cols[0][0].greenF()
        + x*(1-y) * cols[1][0].greenF()
        + y*(1-x) * cols[0][1].greenF()
        + x*y * cols[1][1].greenF();
}

inline
float interpB(const QColor cols[2][2], float x, float y)
{
    return (1-x)*(1-y) * cols[0][0].blueF()
        + x*(1-y) * cols[1][0].blueF()
        + y*(1-x) * cols[0][1].blueF()
        + x*y * cols[1][1].blueF();
}

QColor Interpolate_sV::interpolate(const QImage& in, float x, float y)
{
#ifdef DEBUG_I
    if (x >= in.width()-1 || y >= in.height()-1) {
        Q_ASSERT(false);
    }
#endif
    QColor carr[2][2];
    int floorX = floor(x);
    int floorY = floor(y);
    carr[0][0] = QColor(in.pixel(floorX, floorY));
    carr[0][1] = QColor(in.pixel(floorX, floorY+1));
    carr[1][0] = QColor(in.pixel(floorX+1, floorY));
    carr[1][1] = QColor(in.pixel(floorX+1, floorY+1));

    float dx = x - floorX;
    float dy = y - floorY;
    QColor out = QColor::fromRgbF(
                                  interpR(carr, dx, dy),
                                  interpG(carr, dx, dy),
                                  interpB(carr, dx, dy)
                                 );
    return out;
}

/// validated. correct.
QColor Interpolate_sV::blend(const QColor &left, const QColor &right, float pos)
{
    Q_ASSERT(pos >= 0 && pos <= 1);

    float r = (1-pos)*left.redF()   + pos*right.redF();
    float g = (1-pos)*left.greenF() + pos*right.greenF();
    float b = (1-pos)*left.blueF()  + pos*right.blueF();
    float a = (1-pos)*left.alphaF() + pos*right.alphaF();
    r = CLAMP(r,0.0,1.0);
    g = CLAMP(g,0.0,1.0);
    b = CLAMP(b,0.0,1.0);
    a = CLAMP(a,0.0,1.0);
    return QColor::fromRgbF(r, g, b, a);
}

void Interpolate_sV::blend(ColorMatrix4x4 &c, const QColor &blendCol, float posX, float posY)
{
    Q_ASSERT(posX >= 0 && posX <= 1);
    Q_ASSERT(posY >= 0 && posY <= 1);

    if (c.c00.alpha() == 0) { c.c00 = blendCol; }
    else { c.c00 = blend(c.c00, blendCol, std::sqrt((1-posX) * (1-posY))); }

    if (c.c10.alpha() == 0) { c.c10 = blendCol; }
    else { c.c10 = blend(c.c10, blendCol, std::sqrt( posX * (1-posY))); }

    if (c.c01.alpha() == 0) { c.c01 = blendCol; }
    else { c.c01 = blend(c.c01, blendCol, std::sqrt((1-posX) * posY)); }

    if (c.c11.alpha() == 0) { c.c11 = blendCol; }
    else {c.c11 = blend(c.c11, blendCol, std::sqrt(posX * posY)); }
}


void Interpolate_sV::twowayFlow(const QImage &left, const QImage &right, const FlowField_sV *flowForward, const FlowField_sV *flowBackward, float pos, QImage &output)
{
#ifdef INTERPOLATE
    const float Wmax = left.width()-1.0001; // A little less than the maximum pixel to avoid out of bounds when interpolating
    const float Hmax = left.height()-1.0001;
    float posX, posY;
#endif

    QColor colOut, colLeft, colRight;
    float r,g,b;
    Interpolate_sV::Movement forward, backward;

    for (int y = 0; y < left.height(); y++) {
        for (int x = 0; x < left.width(); x++) {
            forward.moveX = flowForward->x(x, y);
            forward.moveY = flowForward->y(x, y);

            backward.moveX = flowBackward->x(x, y);
            backward.moveY = flowBackward->y(x, y);

#ifdef INTERPOLATE
            posX = x - pos*forward.moveX;
            posY = y - pos*forward.moveY;
            posX = CLAMP(posX, 0, Wmax);
            posY = CLAMP(posY, 0, Hmax);
            colLeft = interpolate(left, posX, posY);

            posX = x - (1-pos)*backward.moveX;
            posY = y - (1-pos)*backward.moveY;
            posX = CLAMP(posX, 0, Wmax);
            posY = CLAMP(posY, 0, Hmax);
            colRight = interpolate(right, posX, posY);
#else
            colLeft = QColor(left.pixel(x - pos*forward.moveX, y - pos*forward.moveY));
            colRight = QColor(right.pixel(x - (1-pos)*backward.moveX , y - (1-pos)*backward.moveY));
#endif
            r = (1-pos)*colLeft.redF() + pos*colRight.redF();
            g = (1-pos)*colLeft.greenF() + pos*colRight.greenF();
            b = (1-pos)*colLeft.blueF() + pos*colRight.blueF();
            colOut = QColor::fromRgbF(
                                      CLAMP1(r),
                                      CLAMP1(g),
                                      CLAMP1(b)
                                      );
            output.setPixel(x,y, colOut.rgb());
        }
    }
}


void Interpolate_sV::newTwowayFlow(const QImage &left, const QImage &right,
                                   const FlowField_sV *flowLeftRight, const FlowField_sV *flowRightLeft,
                                   float pos, QImage &output)
{
    const int W = left.width();
    const int H = left.height();


    SourceField_sV leftSourcePixel(flowLeftRight, pos);
    leftSourcePixel.inpaint();
    SourceField_sV rightSourcePixel(flowRightLeft, 1-pos);
    rightSourcePixel.inpaint();

    float aspect = 1 - (.5 + std::cos(M_PI*pos)/2);

#if defined(FIX_FLOW)
    FlowField_sV diffField(flowLeftRight->width(), flowLeftRight->height());
    FlowTools_sV::difference(*flowLeftRight, *flowRightLeft, diffField);
    float diffSum;
    float tmpAspect;
#endif

#ifdef FIX_BORDERS
    bool leftOk;
    bool rightOk;
#endif


    float fx, fy;
    QColor colLeft, colRight;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

#ifdef FIX_BORDERS
            fx = leftSourcePixel.at(x,y).fromX;
            fy = leftSourcePixel.at(x,y).fromY;
            if (fx >= 0 && fx < W-1
                    && fy >= 0 && fy < H-1) {
                colLeft = interpolate(left, fx, fy);
                leftOk = true;
            } else {
                fx = leftSourcePixel.at(x,y).fromX;
                fy = leftSourcePixel.at(x,y).fromY;
                fx = CLAMP(fx, 0, W-1.01);
                fy = CLAMP(fy, 0, H-1.01);
                colLeft = interpolate(left, fx, fy);
                leftOk = false;
            }

            fx = rightSourcePixel.at(x,y).fromX;
            fy = rightSourcePixel.at(x,y).fromY;
            if (fx >= 0 && fx < W-1
                    && fy >= 0 && fy < H-1) {
                colRight = interpolate(right, fx, fy);
                rightOk = true;
            } else {
                colRight = qRgb(0,255,0);
                rightOk = false;
            }

            if (leftOk && rightOk) {
                output.setPixel(x,y, blend(colLeft, colRight, aspect).rgba());
            } else if (rightOk) {
                output.setPixel(x,y, colRight.rgba());
//                output.setPixel(x,y, qRgb(255, 0, 0));
            } else if (leftOk) {
                output.setPixel(x,y, colLeft.rgba());
//                output.setPixel(x,y, qRgb(0, 255, 0));
            } else {
                output.setPixel(x,y, colLeft.rgba());
            }
#else
            fx = leftSourcePixel.at(x,y).fromX;
            fy = leftSourcePixel.at(x,y).fromY;
            fx = CLAMP(fx, 0, W-1.01);
            fy = CLAMP(fy, 0, H-1.01);
            colLeft = interpolate(left, fx, fy);

#ifdef FIX_FLOW
            diffSum = diffField.x(fx, fy)+diffField.y(fx, fy);
            if (diffSum > 5) {
                tmpAspect = 0;
            } else if (diffSum < -5) {
                tmpAspect = 1;
            } else {
                tmpAspect = aspect;
            }
#endif

            fx = rightSourcePixel.at(x,y).fromX;
            fy = rightSourcePixel.at(x,y).fromY;
            fx = CLAMP(fx, 0, W-1.01);
            fy = CLAMP(fy, 0, H-1.01);
            colRight = interpolate(right, fx, fy);

#ifdef FIX_FLOW
            diffSum = diffField.x(fx, fy)+diffField.y(fx, fy);
            if (diffSum < 5) {
                tmpAspect = 0;
            } else if (diffSum > -5) {
                tmpAspect = 1;
            }
#endif

#ifdef FIX_FLOW
            output.setPixel(x,y, blend(colLeft, colRight, tmpAspect).rgba());
#else
            output.setPixel(x,y, blend(colLeft, colRight, aspect).rgba());
#endif

#endif
        }
    }
}

void Interpolate_sV::forwardFlow(const QImage &left, const FlowField_sV *flow, float pos, QImage &output)
{
    qDebug() << "Interpolating flow at offset " << pos;
#ifdef INTERPOLATE
    float posX, posY;
    const float Wmax = left.width()-1.0001;
    const float Hmax = left.height()-1.0001;
#endif

    QColor colOut;
    Interpolate_sV::Movement forward;    

    for (int y = 0; y < left.height(); y++) {
        for (int x = 0; x < left.width(); x++) {
            // Forward flow from the left to the right image tells for each pixel in the right image
            // from which location in the left image the pixel has come from.
            forward.moveX = flow->x(x, y);
            forward.moveY = flow->y(x, y);


            posX = x - pos*forward.moveX;
            posY = y - pos*forward.moveY;
	        posX = CLAMP(posX, 0, Wmax);
	        posY = CLAMP(posY, 0, Hmax);
#ifdef INTERPOLATE
	        colOut = interpolate(left, posX, posY);
#else
            colOut = QColor(left.pixel(posX, posY));
#endif
	    output.setPixel(x,y, colOut.rgb());
	}
    }
}

void Interpolate_sV::newForwardFlow(const QImage &left, const FlowField_sV *flow, float pos, QImage &output)
{
    const int W = left.width();
    const int H = left.height();

    // Calculate the source flow field
    SourceField_sV field(flow, pos);
    field.inpaint();

    // Draw the pixels
    float fx, fy;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            // Since interpolate() uses the floor()+1 values,
            // set the maximum to a little less than size-1
            // such that the pixel always lies inside.
            fx = field.at(x,y).fromX;
            fx = CLAMP(fx, 0, W-1.01);
            fy = field.at(x,y).fromY;
            fy = CLAMP(fy, 0, H-1.01);
            output.setPixel(x,y, interpolate(left, fx, fy).rgba());
        }
    }
}


/**
  \todo fix bézier interpolation
  \code
      C prev
     /   /
    /   /
   /   /
  A curr
   \
    \
     B next (can be NULL)
  \endcode
  */
void Interpolate_sV::bezierFlow(const QImage &prev, const QImage &right, const FlowField_sV *flowPrevCurr, const FlowField_sV *flowCurrNext, float pos, QImage &output)
{
    const float Wmax = prev.width()-1.0001;
    const float Hmax = prev.height()-1.0001;

    Vector_sV a, b, c;
    Vector_sV Ta, Sa;
    float dist;

    QColor colOut;

    for (int y = 0; y < prev.height(); y++) {
        for (int x = 0; x < prev.width(); x++) {

            a = Vector_sV(x, y);
            // WHY minus?
            c = a + Vector_sV(flowPrevCurr->x(x, y), flowPrevCurr->y(x, y));
            if (flowCurrNext != NULL) {
                b = a + Vector_sV(flowCurrNext->x(x,y), flowCurrNext->y(x,y));
            } else {
                b = a;
            }

            dist = (b-a).length() + (c-a).length();
            if (dist > 0) {
                Ta = b + ( (b-a).length() / dist ) * (c-b);
                Sa = (Ta - a).rotate90();
                Sa = a + Sa;

            } else {
                Sa = a;
            }
#ifdef DEBUG_I
            Sa = a;
#endif

            QPointF position = BezierTools_sV::interpolate(pos, c.toQPointF(), c.toQPointF(), Sa.toQPointF(), a.toQPointF());
            position.rx() = x - pos*flowPrevCurr->x(x,y);
            position.ry() = y - pos*flowPrevCurr->y(x,y);
            position.rx() = CLAMP(position.x(), 0, Wmax);
            position.ry() = CLAMP(position.y(), 0, Hmax);

#ifdef DEBUG_I
//            if (x == 100 && y == 100 && false) {
//                qDebug() << "Interpolated from " << toString(c.toQPointF()) << ", " << toString(a.toQPointF()) << ", "
//                         << toString(b.toQPointF()) << " at " << pos << ": " << toString(position);
//            }
            if (y % 4 == 0) {
                position.rx() = x;
                position.ry() = y;
            }
#endif

            colOut = interpolate(prev, position.x(), position.y());

#ifdef DEBUG_I
            if (y % 4 == 1 && x % 2 == 0) {
                colOut = right.pixel(x, y);
            }
#endif
            output.setPixel(x,y, colOut.rgb());

        }
    }

    /*
    for (int y = 0; y < prev.height(); y++) {
        for (int x = 1; x < prev.width()-1; x++) {
            if (qAlpha(output.pixel(x,y)) == 0
                    && qAlpha(output.pixel(x-1,y)) > 0
                    && qAlpha(output.pixel(x+1,y)) > 0) {
                output.setPixel(x,y, qRgba(
                                    (qRed(output.pixel(x-1,y)) + qRed(output.pixel(x+1,y)))/2,
                                    (qGreen(output.pixel(x-1,y)) + qGreen(output.pixel(x+1,y)))/2,
                                    (qBlue(output.pixel(x-1,y)) + qBlue(output.pixel(x+1,y)))/2,
                                    (qAlpha(output.pixel(x-1,y)) + qAlpha(output.pixel(x+1,y)))/2
                                    ));
            }
        }
    }
    for (int x = 0; x < prev.width(); x++) {
        for (int y = 1; y < prev.height()-1; y++) {
            if (qAlpha(output.pixel(x,y)) == 0
                    && qAlpha(output.pixel(x,y-1)) > 0
                    && qAlpha(output.pixel(x,y+1)) > 0) {
                output.setPixel(x,y, qRgba(
                                    (qRed(output.pixel(x,y-1)) + qRed(output.pixel(x,y+1)))/2,
                                    (qGreen(output.pixel(x,y-1)) + qGreen(output.pixel(x,y+1)))/2,
                                    (qBlue(output.pixel(x,y-1)) + qBlue(output.pixel(x,y+1)))/2,
                                    (qAlpha(output.pixel(x,y-1)) + qAlpha(output.pixel(x,y+1)))/2
                                    ));
            }
        }
    }
    */
}

/**
 * simple linear in time itnerpolation
 */
void Interpolate_sV::simpleinterpolate(const QImage &prev, const QImage &right, float pos, QImage &output)
{

  QColor colOut;

    for (int y = 0; y < prev.height(); y++) {
        for (int x = 0; x < prev.width(); x++) {
                QRgb lt = prev.pixel(x,y);
                QRgb rt = right.pixel(x,y);

                int red = CLAMP((1-pos)*qRed(lt)+(pos)*qRed(rt),0,255);
                int green = CLAMP((1-pos)*qGreen(lt)+(pos)*qGreen(rt),0,255);
                int blue = CLAMP((1-pos)*qBlue(lt)+(pos)*qBlue(rt),0,255);

                QColor out = QColor::fromRgb(red,green,blue);
                                  
                output.setPixel(x,y, out.rgb());
	   } /* for x */
    } /* for y */
}

/**
 * simple nearest frame interoplation
 */
void Interpolate_sV::nearestinterpolate(const QImage &prev, const QImage &right, float pos, QImage &output)
{

    if (pos<0.5) 
            output = prev;
    else
            output = right;  
}
