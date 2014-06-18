#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QtCore/QSettings>
#include <QButtonGroup>

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
    QButtonGroup m_flowMethodGroup;
    QSettings m_settings;
    
private slots:
    void slotValidateFlowBinary();
    void slotUpdateFlowMethod();
    void slotUpdateFfmpeg();
    void slotBrowseFlow();
    void slotBrowseFfmpeg();
};

#endif // PREFERENCESDIALOG_H
