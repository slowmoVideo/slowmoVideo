#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>

namespace Ui {
    class Canvas;
}

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = 0);
    ~Canvas();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::Canvas *ui;
    unsigned int distLeft;
    unsigned int distBottom;
    unsigned int t0x;
    unsigned int t0y;
};

#endif // CANVAS_H
