#ifndef TESTINTMATRIX_SV_H
#define TESTINTMATRIX_SV_H

#include <QObject>
#include <QtTest/QtTest>
class IntMatrix_sV;

class TestIntMatrix_sV : public QObject
{
    Q_OBJECT

private:
    void testAdd(int w, int h, int c);
    static void dumpMatrix(IntMatrix_sV *mat);

private slots:
    void testInitMatrix();
    void testAddMatrix();
    void testAddMatrix2C();
};

#endif // TESTINTMATRIX_SV_H
