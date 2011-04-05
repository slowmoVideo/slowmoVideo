#include "canvas.h"
#include "ui_canvas.h"

#include <QColor>
#include <QImage>
#include <QPainter>

Canvas::Canvas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Canvas),
    distLeft(50),
    distBottom(50),
    t0x(0),
    t0y(0)
{
    ui->setupUi(this);
}

Canvas::~Canvas()
{
    delete ui;
}

void Canvas::paintEvent(QPaintEvent *)
{
    QPainter davinci(this);
    QImage im(this->size(), QImage::Format_ARGB32);
    im.fill(QColor(50, 50, 60).rgb());
    davinci.drawImage(0, 0, im);
}
