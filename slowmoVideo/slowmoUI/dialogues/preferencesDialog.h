#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QtCore/QSettings>

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

protected slots:
    void accept();

private:
    Ui::PreferencesDialog *ui;
    QSettings m_settings;

private slots:
    bool slotValidateFlowBinary();
    void slotBrowseFlow();
};

#endif // PREFERENCESDIALOG_H
