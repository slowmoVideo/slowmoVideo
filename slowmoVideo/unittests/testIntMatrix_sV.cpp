#include "testIntMatrix_sV.h"
#include "../lib/intMatrix_sV.h"
#include <iostream>
#include <iomanip>

void TestIntMatrix_sV::testAdd(int w, int h, int c)
{
    int len = w*h*c;
    unsigned char *orig = new unsigned char[len];
    for (int i = 0; i < len; i++) {
        orig[i] = i;
    }

    unsigned char *data;
    IntMatrix_sV mat(w, h, c);

    mat += orig;
//    dumpMatrix(&mat);
    data = mat.toBytesArray();
    for (int i = 0; i < mat.width()*mat.height()*mat.channels(); i++) {
        QVERIFY(data[i] == orig[i]);
    }
    delete data;

    mat += orig;
//    dumpMatrix(&mat);
    data = mat.toBytesArray();
    for (int i = 0; i < mat.width()*mat.height()*mat.channels(); i++) {
        QVERIFY(data[i] == 2*orig[i]);
    }
    delete data;

    mat /= 2;
//    dumpMatrix(&mat);
    data = mat.toBytesArray();
    for (int i = 0; i < mat.width()*mat.height()*mat.channels(); i++) {
        QVERIFY(data[i] == orig[i]);
    }
    delete data;
}

void TestIntMatrix_sV::testAddMatrix()
{
    testAdd(2,2,1);
}

void TestIntMatrix_sV::testAddMatrix2C()
{
    testAdd(2,2,2);
}

void TestIntMatrix_sV::testInitMatrix()
{
    IntMatrix_sV mat(2, 2, 1);
    unsigned char *data = mat.toBytesArray();
    for (int i = 0; i < mat.width()*mat.height()*mat.channels(); i++) {
        QVERIFY(data[i] == 0);
    }
    delete data;
}

void TestIntMatrix_sV::dumpMatrix(IntMatrix_sV *mat)
{
    std::cout << "Matrix dump: " << std::endl;
    for (int y = 0; y < mat->height(); y++) {
        for (int x = 0; x < mat->width(); x++) {
            if (mat->channels() > 1) {
                std::cout << "[ ";
                for (int i = 0; i < mat->channels(); i++) {
                    std::cout << std::setw(4) << mat->data()[mat->channels()*(y*mat->width()+x) + i];
                }
                std::cout << " ] ";
            } else {
                std::cout << std::setw(4) << mat->data()[y*mat->width()+x];
            }
        }
        std::cout << std::endl;
    }
}
