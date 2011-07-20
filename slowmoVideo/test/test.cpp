#include <iostream>

#include "../project/project_sV.h"
#include "../project/xmlProjectRW_sV.h"
#include "../project/videoFrameSource_sV.h"
#include "../lib/bezierTools_sV.h"

#include <QtGui/QImage>
#include <QtGui/QPainter>


void testSave()
{
    Project_sV proj("/tmp");
    proj.loadFrameSource(new VideoFrameSource_sV(&proj, "/data/Videos/2010-09-14-DSC_5111.AVI"));
    XmlProjectRW_sV writer;
    writer.saveProject(&proj, "/tmp/test.sVproj");
}

void testBezier()
{
    QPointF p0(0,1);
    QPointF p1(2,3);
    QPointF p2(0,0);
    QPointF p3(3,1);

    QImage img(300, 200, QImage::Format_ARGB32);
    img.fill(qRgb(255,255,255));
    for (int i = 0; i < 300; i++) {
        QPointF p = BezierTools_sV::interpolate(float(i)/300, p0, p1, p2, p3);
        img.setPixel(qRound(100*p.x()), qRound(100*p.y()), qRgb(200, 200, 40));
    }
    img.save("/tmp/bezier.png");

    img.fill(qRgb(255,255,255));
    for (int i = 0; i < 300; i++) {
        QPointF p = BezierTools_sV::interpolateAtX(float(i)/100, p0, p1, p2, p3);
        img.setPixel(qRound(100*p.x()), qRound(100*p.y()), qRgb(200, 200, 40));
        qDebug() << "Painting at " << toString(p*100);
        if (qRound(100*p.x()) != i) {
            qDebug() << "this index is off!" << int(100*p.x()) << " != i: " << i << ", qRound: " << qRound(100*p.x());
        }
    }
    img.save("/tmp/bezier2.png");

    img.fill(qRgb(255,255,255));
    QPainter davinci(&img);
    davinci.setRenderHint(QPainter::Antialiasing, true);
    int n = 30;
    QPointF prev = p0*100;
    for (int i = 0; i < n; i++) {
        QPointF cur = BezierTools_sV::interpolateAtX(3*float(i)/n, p0, p1, p2, p3)*100;
        davinci.drawLine(QPointF(prev.x(), prev.y()), QPointF(cur.x(), cur.y()));
        prev = cur;
    }
    davinci.drawLine(QPointF(prev.x(), prev.y()), QPointF(100*p3.x(), 100*p3.y()));
    img.save("/tmp/bezier3.png");

}

void testFloatArg()
{
    qDebug() << QString("%2").arg(24.249, 8, 'f', 2, '0');
}

int main()
{
    testFloatArg();
}
