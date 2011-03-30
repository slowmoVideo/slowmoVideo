#include <iostream>

#include <QString>

#include <QImage>
#include <QColor>
#include <QRgb>

int main(int argc, char *argv[])
{
    if (argc <= 3) {
	std::cout << "Usage: " << argv[0] << " <left image> <right image> <flow image> [<output image>]" << std::endl;
	return 0;
    }
    QImage left(argv[1]);
    QImage right(argv[2]);
    QImage flow(argv[3]);
    flow = flow.scaled(left.size());
    QImage output(left.size(), QImage::Format_RGB32);

    unsigned int steps = 5;
    float pos;

    QRgb pxLeft, pxRight, pxFlow;
    QColor colFlow;
    float moveX, moveY, mult;

    for (int step = 0; step < steps; step++) {
	pos = step/float(steps);
	std::cout << "Pos: " << pos << std::endl;
	for (int x = 0; x < left.width(); x++) {
	    for (int y = 0; y < left.height(); y++) {
		colFlow = QColor(flow.pixel(x,y));
		mult = colFlow.blue();
		moveX = 2 * mult * (colFlow.redF() - .5);
		moveY = 2 * mult * (colFlow.greenF() - .5);
		
		output.setPixel(x, y, left.pixel(x - pos*moveX, y - pos*moveY));
	    }
	}
	output.save(QString("output%1.png").arg(step));
    }
    right.save(QString("output%1.png").arg(steps));

    /*
    if (argc > 4) {
	output.save(argv[4]);
    } else {
	output.save("output.png");
    }
    */
}
