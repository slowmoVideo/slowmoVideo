/*
slowmoFlowEdit is a user interface for editing slowmoVideo's Optical Flow files.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef FLOWEDITCANVAS_H
#define FLOWEDITCANVAS_H

#include <QWidget>
#include <QtCore/QRectF>

class FlowField_sV;
namespace Ui {
    class FlowEditCanvas;
}

/// \todo Auto-fix feature (confirm to accept)
class FlowEditCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit FlowEditCanvas(QWidget *parent = 0);
    ~FlowEditCanvas();

    void setAmplification(float val);
    float amplification() const;
	
	
public slots:
    void slotLoadFlow(QString filename);
    void slotSaveFlow(QString filename = QString());
	void newAmplification(int val);
	
private:
    Ui::FlowEditCanvas *ui;

    FlowField_sV *m_flowField;
    QString m_flowFilename;
    float m_boost;

    void repaintFlow();

private slots:
    void slotRectDrawn(QRectF imageRect);
    void slotExamineValues(float x, float y);
};

#endif // FLOWEDITCANVAS_H
