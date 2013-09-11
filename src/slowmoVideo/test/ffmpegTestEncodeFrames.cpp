
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
#include "../lib/ffmpegEncode_sV.h"
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
    int ret = prepare(&video, "/tmp/ffmpegEncodedFrames.mov", "ffv1", width, height, 1000000, 1, 25);
    if (ret != 0) {
        qDebug() << "Preparing the video failed: " << video.errorMessage << "(" << ret << ")";
        return ret;
    }


    eatSample(&video);
    for (int i = 0; i < 100; i++) {

        img.fill(QColor(100, 100, 100).rgb());
        davinci.drawLine(0, 2*i, width-1, 2*i);
        davinci.fillRect(0, 0, 100, 2*i, QColor(255, 0, 0));

        eatARGB(&video, img.bits());
    }
    finish(&video);
}
