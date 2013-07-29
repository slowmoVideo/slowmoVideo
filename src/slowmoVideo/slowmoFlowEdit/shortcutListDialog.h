/*
slowmoFlowEdit is a user interface for editing slowmoVideo's Optical Flow files.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SHORTCUTLISTDIALOG_H
#define SHORTCUTLISTDIALOG_H

#include <QDialog>
#include <QtCore/QTime>

namespace Ui {
    class ShortcutListDialog;
}
class MainWindow;

class ShortcutListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutListDialog(MainWindow *parent);
    ~ShortcutListDialog();

protected:
    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

private:
    Ui::ShortcutListDialog *ui;

    QTime m_openedAt;
};

#endif // SHORTCUTLISTDIALOG_H
