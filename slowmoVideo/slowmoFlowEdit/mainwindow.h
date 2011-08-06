#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

class FlowEditCanvas;
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    FlowEditCanvas *canvas;

    QSettings m_settings;
    QString m_lastFlowFile;

    void updateTitle();
    void loadFlow(QString filename);

    QString nextFilename(QString originalName, int shift) const;

private slots:
    void slotOpenFlow();
    void slotSaveFlow();

    void slotNextFile();
    void slotPrevFile();
    void slotChangeFile(int shift);
};

#endif // MAINWINDOW_H
