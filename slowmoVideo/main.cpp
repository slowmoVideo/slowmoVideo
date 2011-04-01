#include <iostream>
#include <cmath>

#include <QString>

#include <QImage>
#include <QColor>
#include <QRgb>

#include <QDebug>

#define CLAMP1(x) ( ((x) > 1.0) ? 1.0 : (x) )
#define CLAMP(x,min,max) (  ((x) < (min)) ? (min) : ( ((x) > (max)) ? (max) : (x) )  )

#define INTERPOLATE

enum ColorComponent { CC_Red, CC_Green, CC_Blue };

struct Movement {
    float moveX;
    float moveY;
    float mult;
};

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

int main(int argc, char *argv[])
{
    if (argc <= 4) {
	std::cout << "Usage: " << argv[0] << " <left image> <right image> <flow image> <reverse image> [<output pattern>]" << std::endl;
	return 0;
    }
    QImage left(argv[1]);
    QImage right(argv[2]);
    QImage flow(argv[3]);
    QImage flowBack(argv[4]);
    QImage output(left.size(), QImage::Format_RGB32);

    const int W = left.width();
    const int H = left.height();
    const int Wmax = W-1;
    const int Hmax = H-1;

    if (flow.size() != left.size()) {
	qDebug() << "Re-scaling forward flow image from " << flow.width() << "x" << flow.height() << " to " << left.width() << "x" << left.height();
	flow = flow.scaled(left.size());
    }
    if (flowBack.size() != left.size()) {
	qDebug() << "Re-scaling backward flow image from " << flow.width() << "x" << flow.height() << " to " << left.width() << "x" << left.height();
	flowBack = flowBack.scaled(left.size());
    }


    unsigned int steps = 50;
    float pos;

    QColor colFlow, colOut, colLeft, colRight;
    float r,g,b;
    float posX, posY;
    Movement forward, backward;

    QString pattern("output%1.png");
    if (argc > 5) {
	pattern = QString(argv[5]);
	if (!pattern.contains("%1")) {
	    std::cout << "Error: Output pattern must contain a %1 for the image number. Example: output%1.png." << std::endl;
	    return 0;
	}
    }

    for (unsigned int step = 0; step < steps+1; step++) {
	pos = step/float(steps);
	for (int x = 0; x < left.width(); x++) {
	    for (int y = 0; y < left.height(); y++) {
		colFlow = QColor(flow.pixel(x,y));
		forward.mult = colFlow.blue();
		forward.moveX = 2 * forward.mult * (colFlow.redF() - .5);
		forward.moveY = 2 * forward.mult * (colFlow.greenF() - .5);

		colFlow = QColor(flowBack.pixel(x,y));
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
		//output.setPixel(x,y, colRight.rgb());
	    }
	}
	qDebug() << "Saving position " << pos << " to image " << pattern.arg(step);
	output.save(pattern.arg(step));
    }
    //right.save(pattern.arg(steps));

    /*
    if (argc > 4) {
	output.save(argv[4]);
    } else {
	output.save("output.png");
    }
    */
}
