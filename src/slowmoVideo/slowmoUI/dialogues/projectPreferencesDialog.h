#ifndef PROJECTPREFERENCESDIALOG_H
#define PROJECTPREFERENCESDIALOG_H

#include <QDialog>
#include "project/projectPreferences_sV.h"

namespace Ui {
    class ProjectPreferencesDialog;
}

class ProjectPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectPreferencesDialog(ProjectPreferences_sV *prefs, QWidget *parent = 0);
    ~ProjectPreferencesDialog();

protected:
    void accept();

private:
    Ui::ProjectPreferencesDialog *ui;
    ProjectPreferences_sV *m_projectPrefs;

private slots:
    void slotCheckFPS();
};

#endif // PROJECTPREFERENCESDIALOG_H
