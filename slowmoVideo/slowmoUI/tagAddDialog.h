/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef TAGADDDIALOG_H
#define TAGADDDIALOG_H

#include "../project/tag_sV.h"
#include "../lib/defs_sV.hpp"
#include <QDialog>

namespace Ui {
    class TagAddDialog;
}

class TagAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TagAddDialog(TagAxis defaultAxis, QWidget *parent = 0);
    ~TagAddDialog();
    QString m_text;

    Tag_sV buildTag(QPointF time);

protected:
    void keyPressEvent(QKeyEvent *);

private:
    Ui::TagAddDialog *ui;
    TagAxis m_axis;

private slots:
    void slotTextChanged(const QString& text);
    void slotUpdateAxis();
};

#endif // TAGADDDIALOG_H
