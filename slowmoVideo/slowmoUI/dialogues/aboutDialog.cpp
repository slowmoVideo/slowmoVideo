/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "aboutDialog.h"
#include "ui_aboutDialog.h"
#include "lib/defs_sV.hpp"
#include <QtGui/QImage>
#include <QtGui/QPainter>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog),
    m_appIcon(":icons/slowmoIcon.png")
{
    ui->setupUi(this);
    ui->lblVersion->setText(QString("Version %1, %2").arg(Version_sV::version).arg(Version_sV::bits));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::keyPressEvent(QKeyEvent *)
{
    accept();
}

void AboutDialog::paintEvent(QPaintEvent *e)
{
    QDialog::paintEvent(e);
    QImage img = m_appIcon.scaled(ui->iconFrame->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPainter p(this);
    p.drawImage(ui->iconFrame->pos(), img);
}
