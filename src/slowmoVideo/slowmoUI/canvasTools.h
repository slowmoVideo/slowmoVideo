/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef CANVASTOOLS_H
#define CANVASTOOLS_H

#include <QtCore/QString>

class Canvas;
class Node_sV;
class Project_sV;
class CanvasTools
{
public:
    /// Assembles the output time label at cursor position, taking into account fps and axis resolution.
    static QString outputTimeLabel(Canvas *canvas, Node_sV &time);
    /// Calculates the replay speed at mouse position in percent
    static QString outputSpeedLabel(Node_sV &time, Project_sV *project);
};

#endif // CANVASTOOLS_H
