#include "interpolateSV.h"

#include <cmath>

#include <QImage>
#include <QColor>

#define CLAMP1(x) ( ((x) > 1.0) ? 1.0 : (x) )
#define CLAMP(x,min,max) (  ((x) < (min)) ? (min) : ( ((x) > (max)) ? (max) : (x) )  )

#define INTERPOLATE

enum ColorComponent { CC_Red, CC_Green, CC_Blue };



#ifdef INTERPOLATE

float interp(const QColor cols[2][2], float x, float y, ColorComponent component)
{
    float farr[2][2];
    switch (component) {
    case CC_Red:
	farr[0][0] = cols[0][0].redF();
	farr[0][1] = cols[0][1].redF();
	farr[1][0] = cols[1][0].redF();
	farr[1][1] = cols[1][1].redF();
	break;
    case CC_Green:
	farr[0][0] = cols[0][0].greenF();
	farr[0][1] = cols[0][1].greenF();
	farr[1][0] = cols[1][0].greenF();
	farr[1][1] = cols[1][1].greenF();
	break;
    case CC_Blue:
	farr[0][0] = cols[0][0].blueF();
	farr[0][1] = cols[0][1].blueF();
	farr[1][0] = cols[1][0].blueF();
	farr[1][1] = cols[1][1].blueF();
	break;
    }
    float val = (1-x)*(1-y) * farr[0][0]
	+ x*(1-y) * farr[1][0]
	+ y*(1-x) * farr[0][1]
	+ x*y * farr[1][1];
    return val;
}

QColor interpolate(const QImage& in, float x, float y)
{
    QColor carr[2][2];
    carr[0][0] = QColor(in.pixel(floor(x), floor(y)));
    carr[0][1] = QColor(in.pixel(floor(x), ceil(y)));
    carr[1][0] = QColor(in.pixel(ceil(x), floor(y)));
    carr[1][1] = QColor(in.pixel(ceil(x), ceil(y)));

    float dx = x - floor(x);
    float dy = y - floor(y);
    QColor out = QColor::fromRgbF(
				  interp(carr, dx, dy, CC_Red),
				  interp(carr, dx, dy, CC_Green),
				  interp(carr, dx, dy, CC_Blue)
				  );
    return out;
}
#endif


void InterpolateSV::twowayFlow(const QImage& left, const QImage& right, const QImage& flowForward, const QImage& flowBackward, float pos, QImage output)
{
#ifdef INTERPOLATE
    const int Wmax = left.width()-1;
    const int Hmax = left.height()-1;
    float posX, posY;
#endif

    QColor colFlow, colOut, colLeft, colRight;
    float r,g,b;
    InterpolateSV::Movement forward, backward;

    for (int x = 0; x < left.width(); x++) {
	for (int y = 0; y < left.height(); y++) {
	    colFlow = QColor(flowForward.pixel(x,y));
	    forward.mult = colFlow.blue();
	    forward.moveX = 2 * forward.mult * (colFlow.redF() - .5);
	    forward.moveY = 2 * forward.mult * (colFlow.greenF() - .5);

	    colFlow = QColor(flowBackward.pixel(x,y));
	    backward.mult = colFlow.blue();
	    backward.moveX = 2 * backward.mult * (colFlow.redF() - .5);
	    backward.moveY = 2 * backward.mult * (colFlow.greenF() - .5);

#ifdef INTERPOLATE
	    posX = x - pos*forward.moveX;
	    posY = y - pos*forward.moveY;
	    posX = CLAMP(posX, 0, Wmax);
	    posY = CLAMP(posY, 0, Hmax);
	    colLeft = interpolate(left, posX, posY);
	    
	    posX = x - pos*backward.moveX;
	    posY = y - pos*backward.moveY;
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

void forwardFlow(const QImage& left, const QImage& right, const QImage& flow, float pos, QImage output)
{
#ifdef INTERPOLATE
    float posX, posY;
    const int Wmax = left.width()-1;
    const int Hmax = left.height()-1;
#endif

    QColor colFlow, colOut;
    InterpolateSV::Movement forward;    

    for (int x = 0; x < left.width(); x++) {
	for (int y = 0; y < left.height(); y++) {
	    colFlow = QColor(flow.pixel(x,y));
	    forward.mult = colFlow.blue();
	    forward.moveX = 2 * forward.mult * (colFlow.redF() - .5);
	    forward.moveY = 2 * forward.mult * (colFlow.greenF() - .5);

#ifdef INTERPOLATE
	    posX = x - pos*forward.moveX;
	    posY = y - pos*forward.moveY;
	    posX = CLAMP(posX, 0, Wmax);
	    posY = CLAMP(posY, 0, Hmax);
	    colOut = interpolate(left, posX, posY);
#else
	    colOut = QColor(left.pixel(x - pos*forward.moveX, y - pos*forward.moveY));
#endif
	    output.setPixel(x,y, colOut.rgb());
	}
    }
}
