#include "testVector_sV.h"
#include "../lib/vector_sV.h"

void TestVector_sV::testLength()
{
    Vector_sV vec(0, 0, 3, 4);
    QVERIFY(5 == vec.length());
}

void TestVector_sV::testAdd()
{
    Vector_sV vec(0,1,2,3);
    vec = vec + Vector_sV(0, 1, 2, 3);
    QVERIFY(vec == Vector_sV(4,4));

    vec += Vector_sV(-8, -8.5);
    QVERIFY(vec == Vector_sV(-4, -4.5));

    vec -= Vector_sV(-8, -8.5);
    QVERIFY(vec == Vector_sV(4,4));

}

void TestVector_sV::testEqual()
{
    Vector_sV vec(0,1,2,3);
    QVERIFY(vec == Vector_sV(2, 2));
    QVERIFY(!(vec == Vector_sV(2, 2.1)));

    QVERIFY(vec != Vector_sV(2, 2.1));
    QVERIFY(!(vec != Vector_sV(2, 2)));
}

void TestVector_sV::testScale()
{
    Vector_sV vec(1, -1);
    vec *= 42;
    QVERIFY(vec == Vector_sV(42, -42));
    vec = vec * .5;
    QVERIFY(vec == Vector_sV(21, -21));
    vec = .5 * vec;
    QVERIFY(vec == Vector_sV(10.5, -10.5));
}
