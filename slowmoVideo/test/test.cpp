#include <iostream>

#include "../project/project_sV.h"
#include "../project/xmlProjectRW_sV.h"
#include "../project/videoFrameSource_sV.h"
#include "../lib/bezierTools_sV.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtScript/QScriptEngine>


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

void testQtScript()
{
    int argc = 0;
    QCoreApplication app(argc, NULL);
    QScriptEngine engine;
    QScriptValue val = engine.evaluate("1+2");

    QScriptValue fx = engine.evaluate("(function(x, dy) { return Math.PI; })");
    QScriptValueList args;
    args << .5 << 0;
    qDebug() << fx.call(QScriptValue(), args).toString();
}

void testRegex()
{
    QStringList parts;
    QRegExp e("(\\d+)");
    QString str("forward-lambda20.0_1234_2345.sVflow");
    int min = str.indexOf("_");
    int pos = 0;
    int prevPos = 0;
    while ((pos = e.indexIn(str, pos)) != -1) {
        qDebug() << str.mid(prevPos, pos-prevPos);
        parts << str.mid(prevPos, pos-prevPos);

        if (pos > min) {
            parts << QVariant(e.cap(1).toInt()+1).toString();
        } else {
            parts << e.cap(1);
        }
        qDebug() << e.cap(1) << " at " << pos;

        pos += e.matchedLength();
        prevPos = pos;
    }
    qDebug() << str.mid(prevPos);
    parts << str.mid(prevPos);
    qDebug() << "Next: " << parts.join("");
}

int main()
{
    testRegex();
}
