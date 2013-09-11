/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef FLOWRW_SV_H
#define FLOWRW_SV_H

//#include "defs_sV.hpp"
#include <string>

class FlowField_sV;

/**
  \brief Reads and writes Optical Flow fields.

  Binary format description:
  \code
  "flow_sV" 0x1(char) width(int) height(int)
  x0(float) y0(float) x1 y1 x2 y2 ... x[width*height-1] y[width*height-1]
  \endcode
  The number after \c flow_sV describes the file version, this allows to e.g. add compression
  in future. Width and height should match the image resolution. The following flow data
  describes the movement of each pixel in x and y direction.
  \see FlowField_sV
  */
class FlowRW_sV
{
public:

    /// Holds information about a flow file
    struct FlowInfo_sV {
        /// Flow field width
        int width;
        /// Flow field height
        int height;
        /// Flow field version for future changes in the file format
        char version;
        /// Magic number (first few bytes)
        std::string magic;
        /// Should be set to true if the flow is valid (valid magic number, valid version etc.)
        bool valid;
        /// Initially set to invalid.
        FlowInfo_sV() {
            magic = "flow_sV";
            version = '1';
            valid = false;
        }
    };

    struct FlowRWError {
        std::string message;
        FlowRWError(std::string msg) : message(msg) {}
    };

    /** \fn load(std::string)
      \return \c NULL, if the file could not be loaded, and the flow field otherwise.
      */
    /** \fn save(std::string, FlowField_sV*);
      \see FlowField_sV::FlowField_sV(int, int, float*, FlowField_sV::GLFormat)
      */
    /** \fn readInfo(std::string)
      \return Information about the flow file (like dimension); Does not read the whole file and is therefore faster than load(std::string).
      */
    static void save(std::string filename, FlowField_sV *flowField);
    static FlowField_sV* load(std::string filename) throw(FlowRWError);

    static FlowInfo_sV readInfo(std::string filename);

private:
    static const std::string m_magicNumber;
    static const char m_version;
};

#endif // FLOWRW_SV_H
