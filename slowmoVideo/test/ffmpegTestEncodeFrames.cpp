
// Against the «UINT64_C not declared» message.
// See: http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

extern "C" {
#include "ffmpegTestEncode.h"
}

#include <QtCore/QDebug>
#include <QImage>
#include <QPainter>
#include <QColor>

int main()
{
    int width = 640, height = 320;
    QImage img(width, height, QImage::Format_ARGB32);

    QPainter davinci(&img);
    davinci.setPen(QColor(40, 80, 255, 20));

    VideoOut_sV video;
    prepare(&video, width, height, 1000000, 1, 25, true);


    eatSample(&video);
    for (int i = 0; i < 50; i++) {

        img.fill(QColor(100, 100, 100).rgb());
        davinci.drawLine(0, 2*i, width-1, 2*i);
        davinci.fillRect(0, 0, 100, 2*i, QColor(255, 0, 0));

        qDebug() << "Null image? " << img.isNull();

        unsigned char *ptr = img.bits();
        qDebug() << ptr[0] << ptr[1];

        eatARGB(&video, img.bits());
    }
    finish(&video);
}
