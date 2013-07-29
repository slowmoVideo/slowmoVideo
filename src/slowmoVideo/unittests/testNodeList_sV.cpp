#include "testNodeList_sV.h"

#include "../project/nodeList_sV.h"

void TestNodeList_sV::testCurveType()
{
    NodeList_sV nodes;

    nodes.add(Node_sV(0, 1));
    nodes.add(Node_sV(2, 1));
    nodes.add(Node_sV(4, 1));
    nodes.setCurveType(1, CurveType_Bezier);
    QVERIFY(nodes.at(0).rightCurveType() == CurveType_Bezier);
    QVERIFY(nodes.at(1).rightCurveType() == CurveType_Linear);
    nodes.setCurveType(3, CurveType_Bezier);
    QVERIFY(nodes.at(0).rightCurveType() == CurveType_Bezier);
    QVERIFY(nodes.at(1).rightCurveType() == CurveType_Bezier);

    nodes.add(Node_sV(1,1));
    QVERIFY(nodes.at(0).rightCurveType() == CurveType_Linear);
    QVERIFY(nodes.at(1).rightCurveType() == CurveType_Linear);
    QVERIFY(nodes.at(2).rightCurveType() == CurveType_Bezier);
}
