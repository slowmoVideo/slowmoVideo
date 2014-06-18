/*
slowmoFlowEdit is a user interface for editing slowmoVideo's Optical Flow files.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QtCore>
#include <QObject>
#include <QMainWindow>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif
#include <QSettings>

#include <QtCore/QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "flowEditCanvas.h"
#include "shortcutListDialog.h"

#define MAX_SEARCH_SHIFT 500

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_cs(this)
{
    ui->setupUi(this);

    restoreGeometry(m_settings.value("geometry").toByteArray());
    restoreState(m_settings.value("windowState").toByteArray());

    m_canvas = new FlowEditCanvas(this);
    setCentralWidget(m_canvas);
    m_canvas->setAmplification(m_settings.value("view/amplify", 1.0).toFloat());

    m_cs.addShortcut("o", OPEN, "Open flow file");
    m_cs.addShortcut("s-s", SAVE, "Save");
    m_cs.addShortcut("j", PREV, "Previous file");
    m_cs.addShortcut("k", NEXT, "Next file");
    //m_cs.addShortcut("b-1", BOOST1, "No amplification");
    //m_cs.addShortcut("b-2", BOOST2, "Low amplification");
    //m_cs.addShortcut("b-3", BOOST3, "High amplification (details best visible)");
    m_cs.addShortcut("q-q", QUIT, "Quit");
    m_cs.addShortcut("h-h", HELP, "Show shortcut dialog");

    ui->actionQuit->setShortcut(QKeySequence("Ctrl+Q"));
    ui->actionOpen->setShortcut(QKeySequence("Ctrl+O"));
    ui->actionSave->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionPrev->setShortcut(QKeySequence("Ctrl+Left"));
    ui->actionNext->setShortcut(QKeySequence("Ctrl+Right"));
    ui->actionShortcuts->setShortcut(QKeySequence("F1"));

    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(slotOpenFlow()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(slotSaveFlow()));
    connect(ui->actionNext, SIGNAL(triggered()), this, SLOT(slotNextFile()));
    connect(ui->actionPrev, SIGNAL(triggered()), this, SLOT(slotPrevFile()));
    connect(ui->actionShortcuts, SIGNAL(triggered()), this, SLOT(slotShowShortcuts()));
    connect(&m_cs, SIGNAL(signalShortcutUsed(int)), this, SLOT(slotShortcutUsed(int)));


    updateTitle();
    if (m_settings.value("prevFlowFile", "").toString().length() != 0) {
        loadFlow(m_settings.value("prevFlowFile", "").toString());
    }

    qDebug() << "Shortcut list: " << m_cs.shortcutList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

const CombinedShortcuts& MainWindow::shortcuts() const
{
    return m_cs;
}

void MainWindow::updateTitle()
{
    QString file = m_lastFlowFile;
    if (file.length() == 0) {
        file = "no file loaded";
    }
    setWindowTitle(QString("slowmo Flow Editor (%1)").arg(file));
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    m_settings.setValue("geometry", saveGeometry());
    m_settings.setValue("windowState", saveState());
    if (m_lastFlowFile.length() > 0) {
        m_settings.setValue("prevFlowFile", m_lastFlowFile);
    }
    m_settings.setValue("view/amplify", m_canvas->amplification());
    QMainWindow::closeEvent(e);
}

QString MainWindow::nextFilename(QString originalName, int shift) const
{
    if (false) {
        QStringList parts;
        QRegExp e("(\\d+)");
        int min = originalName.indexOf("_");
        int pos = 0;
        int prevPos = 0;
        while ((pos = e.indexIn(originalName, pos)) != -1) {
            parts << originalName.mid(prevPos, pos-prevPos);

            if (pos > min) {
                parts << QVariant(e.cap(1).toInt()+shift).toString();
            } else {
                parts << e.cap(1);
            }

            pos += e.matchedLength();
            prevPos = pos;
        }
        parts << originalName.mid(prevPos);
        return parts.join("");
    } else {
        QStringList filters;
        filters << "*.sVflow";

        QDir dir(QFileInfo(originalName).absolutePath());
        QStringList filenames = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name);

        QString current = QFileInfo(originalName).fileName();
        QString next;
        if (filenames.contains(current)) {
            int index = filenames.indexOf(current);
            if (filenames.size() > index+shift && index+shift >= 0) {
                next = QFileInfo(originalName).absolutePath() + "/" + filenames[index+shift];
            } else {
                qDebug() << "No file in this direction";
            }
        } else {
            qDebug() << filenames;
        }

        return next;
    }
}

void MainWindow::loadFlow(QString filename)
{
    if (QFileInfo(filename).exists()) {
        m_canvas->slotLoadFlow(filename);
        m_lastFlowFile = filename;
        updateTitle();
    }
}

void MainWindow::slotOpenFlow()
{
    QFileDialog dialog(this, "Open flow file");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Flow files (*.sVflow)");
    if (m_settings.value("directories/lastFlowDir", "").toString().length() > 0) {
        dialog.setDirectory(m_settings.value("directories/lastFlowDir", "").toString());
    }
    if (dialog.exec() == QDialog::Accepted) {
        m_settings.setValue("directories/lastFlowDir", QFileInfo(dialog.selectedFiles().at(0)).absolutePath());
        loadFlow(dialog.selectedFiles().at(0));
        statusBar()->showMessage("Loaded " + m_lastFlowFile, 3000);
    }
}

void MainWindow::slotSaveFlow()
{
    statusBar()->showMessage("Saving ...", 3000);
    m_canvas->slotSaveFlow();
    statusBar()->showMessage("Saved " + m_lastFlowFile, 3000);
}

void MainWindow::slotNextFile()
{
    slotChangeFile(+1);
}

void MainWindow::slotPrevFile()
{
    slotChangeFile(-1);
}

void MainWindow::slotChangeFile(int shift)
{
    for (int i = 1; i < MAX_SEARCH_SHIFT; i++) {
        QString name = nextFilename(m_lastFlowFile, i*shift);
        if (QFileInfo(name).exists()) {
            loadFlow(name);
            return;
        }
    }
    QMessageBox::warning(this, "File not found", QString("The flow file %1 does not exist.\n\n "
                                                         "I even searched %2 steps for a file in this direction, "
                                                         "and still did not find a file.")
                         .arg(nextFilename(m_lastFlowFile, shift)).arg(MAX_SEARCH_SHIFT), QMessageBox::Ok);
}

void MainWindow::amplify(float val)
{
    m_canvas->setAmplification(val);
    statusBar()->showMessage(QString("Setting visual amplification to %1").arg(val), 3000);
}

void MainWindow::slotShortcutUsed(int id)
{
/*
    if (id == BOOST1) {
        qDebug() << "Amplify 1";
        amplify(1);
    } else if (id == BOOST2) {
        qDebug() << "Amplify 2";
        amplify(3);
    } else if (id == BOOST3) {
        qDebug() << "Amplify 3";
        amplify(9);
    } else 
    */
    if (id == PREV) {
        slotPrevFile();
    } else if (id == NEXT) {
        slotNextFile();
    } else if (id == OPEN) {
        slotOpenFlow();
    } else if (id == SAVE) {
        slotSaveFlow();
    } else if (id == HELP) {
        slotShowShortcuts();
    } else if (id == QUIT) {
        close();
    } else {
        qDebug() << "Shortcut with ID " << id << " has no action!";
        Q_ASSERT(false);
    }
}

void MainWindow::slotShowShortcuts()
{
    ShortcutListDialog dialog(this);
    dialog.exec();
}
