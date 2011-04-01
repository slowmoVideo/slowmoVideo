#include <iostream>

#include <QString>

#include <QImage>
#include <QColor>
#include <QRgb>

#include <QDebug>

#include "interpolateSV.h"





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
	InterpolateSV::twowayFlow(left, right, flow, flowBack, pos, output);
	qDebug() << "Saving position " << pos << " to image " << pattern.arg(step);
	output.save(pattern.arg(step));
    }
}
