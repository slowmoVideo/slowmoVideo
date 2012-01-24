#ifndef FLOWTOOLS_SV_H
#define FLOWTOOLS_SV_H

#include "flowField_sV.h"
#include "kernel_sV.h"

class FlowTools_sV
{
public:
    enum LineFillMode { HorizontalFromTop, HorizontalFromBottom, HorizontalFromBoth,
                        VerticalFromLeft, VerticalFromRight, VerticalFromBoth };

    enum CornerPosition { TopLeft, TopRight, BottomLeft, BottomRight };

    static void difference(const FlowField_sV &left, const FlowField_sV &right, FlowField_sV &out);
    static void signedDifference(const FlowField_sV &left, const FlowField_sV &right, FlowField_sV &out);

    static void deleteRect(FlowField_sV &field, int top, int left, int bottom, int right);

    /**
      \brief Clears the content of the given rectangle and fills it with the surrounding pixels.

      The coordinates are inclusive, so filling <code>(0,0,1,1)</code> fills four pixels. The axis origin <code>(0|0)</code>
      is at top left.
      */
    static void refill(FlowField_sV &field, int top, int left, int bottom, int right);

    static void refill(FlowField_sV &field, const Kernel_sV &kernel, int top, int left, int bottom, int right);

    static FlowField_sV* median(FlowField_sV const * const fa, FlowField_sV const * const fb, FlowField_sV const * const fc);

private:
    static void refillLine(FlowField_sV &field, int startTop, int startLeft, int length, LineFillMode fillMode);
    static void refillLine(FlowField_sV &field, const Kernel_sV &kernel, int startTop, int startLeft, int length, bool horizontal);
    static void refillCorner(FlowField_sV &field, int top, int left, CornerPosition pos);
};

#endif // FLOWTOOLS_SV_H
