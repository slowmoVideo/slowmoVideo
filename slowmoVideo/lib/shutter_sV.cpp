#include "shutter_sV.h"
#include "intMatrix_sV.h"

#include <QtCore/QStringList>

QImage Shutter_sV::combine(QStringList images)
{
    Q_ASSERT(images.size() > 0);

    QImage img(images.at(0));
    IntMatrix_sV matrix(img.width(), img.height(), 4);
    for (int i = 0; i < images.size(); i++) {
        img = QImage(images.at(i));
        matrix += img.bits();
    }
    matrix /= images.size();

    QImage result(matrix.toBytesArray(), matrix.width(), matrix.height(), QImage::Format_ARGB32);
    return result;
}
