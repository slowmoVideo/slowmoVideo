/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SOURCEFIELD_SV_H
#define SOURCEFIELD_SV_H

class FlowField_sV;

/**
  \brief Counterpart to an Optical Flow field, says where a pixel came from.

  \todo Reversing vectors: Take smallest one?
  */
class SourceField_sV
{
public:
    /**
      \fn SourceField_sV(int width, int height)
      \brief Creates an empty source field.
      */
    /**
      \fn SourceField_sV(const FlowField_sV *flow, float pos)
      \brief Converts the optical flow to «where did the pixel come from?»
      from the target image's perspective.

      The target image is at position \c pos.

      inpaint() is \em not called by this method.
      */
    /**
      \fn inpaint()
      \brief Fills holes in the source field.

      When generating a source field from an optical flow field, for example:
      \code
0  0  1  1  0  0
       |
       v
0  1  1  1  1  0 \endcode
      then the source field might look like this:
      \code
0  1       -1  0 \endcode
      since the two edges moved one pixel left or right, respectively, and no pixel
      «went to» the part in the middle. This function interpolates those from nearby members
      whose source location is known.
      */
    SourceField_sV(const SourceField_sV &other);
    SourceField_sV(int width, int height);
    SourceField_sV(const FlowField_sV *flow, float pos);
    ~SourceField_sV();

    /**
      Represents a source flow pixel.
      For a Source at <code>(x|y)</code>, the pixel at <code>(x|y)</code> originated at <code>(fromX|fromY)</code>.
      This value usually is a float and the colour at its location should therefore be interpolated to avoid aliasing artifacts.
      */
    struct Source {
        /// x coordinate of the origin
        float fromX;
        /// y coordinate of the origin
        float fromY;
        /// \c false if this item is still a «hole» (see inpaint())
        bool isSet;
        /// Creates an empty Source (i.e. a hole)
        Source() : isSet(false) {}
        /// Creates an initialized Source
        Source(float fromX, float fromY) : fromX(fromX), fromY(fromY), isSet(true) {}
        /// Sets the source coordinates. Is not a hole anymore afterwards.
        void set(float x, float y) {
            fromX = x; fromY = y; isSet = true;
        }
    };

    /// Source array. Only public for the inline function at().
    Source *m_field;


    /// Returns the Source at the given coordinates, for calculating the colour at <code>(x|y)</code>.
    inline Source& at(int x, int y)
    {
        return m_field[m_width*y + x];
    }

    void inpaint();
    void fixFlow(const FlowField_sV *flowOtherDirection);

    SourceField_sV& operator =(const SourceField_sV &other);

private:
    int m_width;
    int m_height;

    struct SourceSum {
        float x;
        float y;
        int count;
        SourceSum()
        {
            reset();
        }
        Source norm() {
            if (count == 0) { return Source(); }
            else { return Source(x/count, y/count); }
        }

        void operator +=(const Source &other)
        {
            if (other.isSet) {
                count++;
                x += other.fromX;
                y += other.fromY;
            }
        }
        void reset()
        {
            x = 0;
            y = 0;
            count = 0;
        }
    };

};

#endif // SOURCEFIELD_SV_H
