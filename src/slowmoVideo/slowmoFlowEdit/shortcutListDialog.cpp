/*
slowmoFlowEdit is a user interface for editing slowmoVideo's Optical Flow files.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "shortcutListDialog.h"
#include "ui_shortcutListDialog.h"

#include "mainwindow.h"

ShortcutListDialog::ShortcutListDialog(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutListDialog),
    m_openedAt(QTime::currentTime())
{
    ui->setupUi(this);
    ui->shortcuts->setText(parent->shortcuts().shortcutList());
    adjustSize();
}

ShortcutListDialog::~ShortcutListDialog()
{
    delete ui;
}

void ShortcutListDialog::keyReleaseEvent(QKeyEvent *)
{
    // Ensure the dialog is not closed with the same key it was opened with
    if (m_openedAt.elapsed() > 1000) {
        close();
    }
}
void ShortcutListDialog::mouseReleaseEvent(QMouseEvent *)
{
    if (m_openedAt.elapsed() > 1000) {
        close();
    }
}
