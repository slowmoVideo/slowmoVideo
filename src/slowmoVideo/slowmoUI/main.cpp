/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include <QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{

#ifdef Q_OS_MACX
	// fix for osx UI and Qt4
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif

    QApplication a(argc, argv);

    // Set up preferences for the QSettings file
    QCoreApplication::setOrganizationName("Granjow");
    QCoreApplication::setOrganizationDomain("granjow.net");
    QCoreApplication::setApplicationName("slowmoUI");



    QString projectPath;
    qDebug() << "threading info : " << QThread::idealThreadCount();
    qDebug() << a.arguments();

    const int N = a.arguments().size();
    for (int n = 1; n < N; n++) {
        QString arg = a.arguments().at(n);
        if (arg.startsWith("--")) {

            bool langUpdated = true;

            // Changes the file loaded from the resource container
            // to force a different language
            if (arg == "--de") {
                QLocale::setDefault(QLocale::German);
            } else if (arg == "--en") {
                QLocale::setDefault(QLocale::English);
            } else if (arg == "--it") {
                QLocale::setDefault(QLocale::Italian);
            } else {
                langUpdated = false;
            }

            if (langUpdated) {
                qDebug() << "Changed locale to " << QLocale::languageToString(QLocale().language());
            } else {
                qDebug() << "Not handled: " << arg;
            }

        } else {
            QFileInfo info(arg);
            if (info.exists() && info.isReadable() && info.isFile()) {
                projectPath = info.absoluteFilePath();
                qDebug() << "Loading project: " << projectPath;
            } else {
                qDebug() << projectPath << " does not exist.";
            }
        }
    }

    // Load the translation file from the resource container and use it
    QTranslator translator;
    translator.load(":translations");
    a.installTranslator(&translator);

    MainWindow w(projectPath);

    w.show();

    return a.exec();
}
