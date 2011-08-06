#ifndef FLOWTOOLS_SV_H
#define FLOWTOOLS_SV_H

#include "flowField_sV.h"

class FlowTools_sV
{
public:
    enum LineFillMode { HorizontalFromTop, HorizontalFromBottom, HorizontalFromBoth,
                        VerticalFromLeft, VerticalFromRight, VerticalFromBoth };

    enum CornerPosition { TopLeft, TopRight, BottomLeft, BottomRight };

    static void difference(const FlowField_sV &left, const FlowField_sV &right, FlowField_sV &out);
    static void signedDifference(const FlowField_sV &left, const FlowField_sV &right, FlowField_sV &out);

    /// 0 point is at top left.
    /// (top|left) and (bottom|right) are inclusive.
    static void refill(FlowField_sV &field, int top, int left, int bottom, int right);

    static void refillLine(FlowField_sV &field, int startTop, int startLeft, int length, LineFillMode fillMode);
    static void refillCorner(FlowField_sV &field, int top, int left, CornerPosition pos);
};

#endif // FLOWTOOLS_SV_H
