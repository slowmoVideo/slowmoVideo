#ifndef SHUTTERFUNCTIONDIALOG_H
#define SHUTTERFUNCTIONDIALOG_H

#include <QDialog>
class Project_sV;
class ShutterFunction_sV;

namespace Ui {
    class ShutterFunctionDialog;
}

class ShutterFunctionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShutterFunctionDialog(Project_sV *project, QWidget *parent = 0);
    ~ShutterFunctionDialog();

    void loadProject(Project_sV *project);
    void setSegment(int segment);

public slots:
    void slotNodesUpdated();

protected slots:
    virtual void paintEvent(QPaintEvent *e);

private:
    static QString emptyFunction;

    Ui::ShutterFunctionDialog *ui;
    Project_sV *m_project;

    ShutterFunction_sV *m_currentFunction;

    float m_dy;
    float m_t0;

    int m_segment;

private slots:
    void slotUpdateNode();

    void slotFunctionTextChanged();
    void slotLoadSelectedFunction();

    void slotAddFunction();
    void slotRemoveFunction();

    void slotNextSegment();
    void slotPrevSegment();
};

#endif // SHUTTERFUNCTIONDIALOG_H
