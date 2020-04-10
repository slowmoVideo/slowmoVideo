/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "tagAddDialog.h"
#include "ui_tagAddDialog.h"
#include <QtGui/QKeyEvent>

TagAddDialog::TagAddDialog(TagAxis defaultAxis, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagAddDialog),
    m_axis(defaultAxis)
{
    ui->setupUi(this);
    ui->bOk->setEnabled(false);

    connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->tag, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
    connect(ui->tag, SIGNAL(returnPressed()), ui->bOk, SLOT(click()));

    slotUpdateAxis();
}

TagAddDialog::~TagAddDialog()
{
    delete ui;
}

Tag_sV TagAddDialog::buildTag(QPointF time)
{
    if (m_axis == TagAxis_Source) {
        return Tag_sV(time.y(), ui->tag->text(), m_axis);
    } else {
        return Tag_sV(time.x(), ui->tag->text(), m_axis);
    }
}

void TagAddDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up) {
        m_axis = TagAxis_Source;
        slotUpdateAxis();
    } else if (e->key() == Qt::Key_Down) {
        m_axis = TagAxis_Output;
        slotUpdateAxis();
    } else {
        QDialog::keyPressEvent(e);
    }
}

void TagAddDialog::slotTextChanged(const QString &text)
{
    if (text.length() == 0) {
        ui->bOk->setEnabled(false);
    } else {
        ui->bOk->setEnabled(true);
        m_text = text;
    }
}

void TagAddDialog::slotUpdateAxis()
{
    QSizePolicy::Policy upperPolicy = QSizePolicy::Fixed;
    QSizePolicy::Policy lowerPolicy = QSizePolicy::Expanding;
    if (m_axis == TagAxis_Output) {
        upperPolicy = QSizePolicy::Expanding;
        lowerPolicy = QSizePolicy::Fixed;
    }
    ui->verticalUpperSpacer->changeSize(0, 0, QSizePolicy::Minimum, upperPolicy);
    ui->verticalLowerSpacer->changeSize(0, 0, QSizePolicy::Minimum, lowerPolicy);
    ui->verticalLayout_2->invalidate();

    repaint();
    updateGeometry();
}
