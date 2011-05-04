#ifndef PROGRESSDIALOGBUILDFLOW_H
#define PROGRESSDIALOGBUILDFLOW_H

#include <QDialog>

class QString;
namespace Ui {
    class ProgressDialogBuildFlow;
}

class ProgressDialogBuildFlow : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialogBuildFlow(QWidget *parent = 0);
    ~ProgressDialogBuildFlow();

    void setProgressRange(int value);

public slots:
    void slotFlowAborted();
    void slotFlowFinished();
    void slotProgressUpdated(int max);
    void slotCurrentFile(const QString &name);

private:
    Ui::ProgressDialogBuildFlow *ui;
    int m_progressRange;

private slots:
    void slotAbortPressed();

signals:
    void signalAbortPressed();
};

#endif // PROGRESSDIALOGBUILDFLOW_H
