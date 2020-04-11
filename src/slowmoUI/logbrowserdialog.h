#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>
#include <QTextEdit>

class QPushButton;

class LogBrowserDialog : public QDialog {
Q_OBJECT

public:
    explicit LogBrowserDialog(QWidget *parent = nullptr);

public slots:

    void outputMessage(QtMsgType type, const QString &msg);

protected slots:

    void save();

protected:
    void keyPressEvent(QKeyEvent *e) override;

    QTextEdit *browser;
    QPushButton *clearButton;
    QPushButton *saveButton;
};

#endif // LOGDIALOG_H
