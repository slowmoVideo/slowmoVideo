#ifndef TESTVECTOR_SV_H
#define TESTVECTOR_SV_H

#include <QObject>
#include <QtTest/QtTest>

class TestVector_sV : public QObject
{
    Q_OBJECT

private slots:
    void testLength();
    void testAdd();
    void testScale();

    void testEqual();

};

#endif // TESTVECTOR_SV_H
