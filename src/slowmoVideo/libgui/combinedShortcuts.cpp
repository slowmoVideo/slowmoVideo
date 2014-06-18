/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "combinedShortcuts.h"

#include <QShortcut>

#define DEBUG
#include <QDebug>

CombinedShortcuts::CombinedShortcuts(QWidget *parent) :
    m_parent(parent),
    m_signalMapper(parent)
{
    connect(&m_signalMapper, SIGNAL(mapped(QString)), this, SLOT(slotShortcutUsed(QString)));
}
CombinedShortcuts::~CombinedShortcuts()
{
    for (int i = 0; i < m_qShortcuts.size(); i++) {
        delete m_qShortcuts.at(i);
    }
}

QString CombinedShortcuts::shortcutList() const
{
    QStringList shortcuts;
    for (int i = 0; i < m_shortcuts.size(); i++) {
        shortcuts << m_shortcuts.at(i).shortcut
                     + "\t" +
                     m_shortcuts.at(i).desc;
    }
    return shortcuts.join("\n");
}

void CombinedShortcuts::addShortcut(QString shortcut, int id, QString description)
{
    for (int i = 0; i < m_shortcuts.size(); i++) {
        if (m_shortcuts.at(i).shortcut == shortcut) {
            qDebug() << "Shortcut " << shortcut << " is not unique, used for "
                     << description << " and " << m_shortcuts.at(i).desc;
            Q_ASSERT(false);
            return;
        }
    }
    if (shortcut.length() == 1) {
        addShortcutKey(shortcut);
        m_shortcuts << ShortcutItem(id, shortcut, description);

    } else if (shortcut.length() == 3 && shortcut.at(1) == '-') {
        addShortcutKey(shortcut.at(0));
        addShortcutKey(shortcut.at(2));
        m_shortcuts << ShortcutItem(id, shortcut, description);

    } else {
        qDebug() << "Cannot add shortcut " << shortcut << ", format not supported.";
        Q_ASSERT(false);
    }
}

void CombinedShortcuts::addShortcutKey(QString key)
{
    if (!m_uniqueKeys.contains(key)) {
#ifdef DEBUG
        qDebug() << "Adding unique key " << key;
#endif
        m_uniqueKeys << key;

        // Create a new shortcut for each unique key
        QShortcut *qshortcut = new QShortcut(QKeySequence(key), m_parent);
        m_qShortcuts << qshortcut;

        m_signalMapper.setMapping(qshortcut, key);

        // Connect shortcut to the signal mapper
        connect(qshortcut, SIGNAL(activated()), &m_signalMapper, SLOT(map()));
    }
}

void CombinedShortcuts::slotShortcutUsed(QString key)
{

    TimedShortcut ts;
    ts.shortcut = key;
    ts.start = QTime::currentTime();

#ifdef DEBUG
    qDebug() << key << " pressed. Last shortcut: " << m_previousKey.start.elapsed() << " ms ago.";
#endif

//    QString at = QString(" @ %1.%2::%3")
//            .arg(ts.start.minute())
//            .arg(ts.start.second())
//            .arg(ts.start.msec());

    bool handled = false;

    // Use a timeout. Otherwise pressing a key may lead to unpredictable results
    // since it may depend on the key you pressed several minutes ago.
    if (m_previousKey.start.elapsed() < 600) {

        QString combinedShortcut = QString("%1-%2").arg(m_previousKey.shortcut).arg(key);

        for (int i = 0; i < m_shortcuts.size(); i++) {
            if (m_shortcuts.at(i).shortcut == combinedShortcut) {
#ifdef DEBUG
                qDebug() << QString("Shortcut %1 (%2) has been triggered!")
                            .arg(m_shortcuts.at(i).shortcut)
                            .arg(m_shortcuts.at(i).desc);
#endif
                emit signalShortcutUsed(m_shortcuts.at(i).id);
                handled = true;
                break;
            }
        }

    }
    if (!handled) {
        // The key pressed did not belong to a combined shortcut.
        // Check if there is a shortcut with a single key for it.

        for (int i = 0; i < m_shortcuts.size(); i++) {
            if (m_shortcuts.at(i).shortcut == key) {
                emit signalShortcutUsed(m_shortcuts.at(i).id);
                handled = true;
                break;
            }
        }
    }

    if (!handled) {
        m_previousKey = ts;
    }
}
