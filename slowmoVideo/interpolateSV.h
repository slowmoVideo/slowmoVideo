/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/


class QImage;

class InterpolateSV {

 public:

    struct Movement {
	float moveX;
	float moveY;
	float mult;
    };

    static void forwardFlow(const QImage& left, const QImage& flow, float pos, QImage& output);
    static void twowayFlow(const QImage& left, const QImage& right, const QImage& flowForward, const QImage& flowBackward, float pos, QImage& output);
};
