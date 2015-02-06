/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef COMBINEDSHORTCUTS_H
#define COMBINEDSHORTCUTS_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSignalMapper>
#include <QtCore/QTime>
//#include <QtWidgets>

class QShortcut;

/**
  \brief Manager for combined shortcuts (known from GMail)

  Combined shortcuts are for example <code>s-a</code> or \c q; the former one
  is triggered when the user presses \c s and within a limited amount of time \c a.
  */
class CombinedShortcuts : public QObject
{
    Q_OBJECT
public:
    CombinedShortcuts(QWidget *parent);
    ~CombinedShortcuts();

    /// Adds the given shortcut if it does not exist yet
    void addShortcut(QString shortcut, int id, QString description);

    /// Returns a list of the available shortcuts that have been added
    QString shortcutList() const;


signals:
    /// Emitted when the shortcut has been used
    void signalShortcutUsed(int id);

private:
    struct ShortcutItem {
        ShortcutItem(int id, QString shortcut, QString desc) :
            id(id),
            shortcut(shortcut),
            desc(desc) {}
        int id;
        QString shortcut;
        QString desc;
    };
    struct TimedShortcut {
        QTime start;
        QString shortcut;
    };

    QList<ShortcutItem> m_shortcuts;
    QList<QString> m_uniqueKeys;
    QList<QShortcut*> m_qShortcuts;
    TimedShortcut m_previousKey;

    QWidget *m_parent;
    QSignalMapper m_signalMapper;

    void addShortcutKey(QString key);


private slots:
    void slotShortcutUsed(QString key);
};

#endif // COMBINEDSHORTCUTS_H
