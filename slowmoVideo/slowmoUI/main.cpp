/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include <QtGui/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set up preferences for the QSettings file
    QCoreApplication::setOrganizationName("Granjow");
    QCoreApplication::setOrganizationDomain("granjow.net");
    QCoreApplication::setApplicationName("slowmoUI");

    // Changes the file loaded from the resource container
    if (a.arguments().contains("--de")) {
        QLocale::setDefault(QLocale::German);
    } else if (a.arguments().contains("--en")) {
        QLocale::setDefault(QLocale::English);
    }

    // Load the translation file from the resource container and use it
    QTranslator translator;
    translator.load(":translations");
    a.installTranslator(&translator);

    QString projectPath;
    qDebug() << a.arguments();
    if (a.arguments().size() >= 2) {
        qDebug() << a.arguments().at(1);
        QFileInfo info(a.arguments().at(1));
        if (info.exists() && info.isReadable() && info.isFile()) {
            projectPath = info.absoluteFilePath();
            qDebug() << "Loading project: " << projectPath;
        } else {
            qDebug() << projectPath << " does not exist.";
        }
    } else {
        qDebug() << "No argument given.";
    }

    MainWindow w(projectPath);

    w.show();

    return a.exec();
}
