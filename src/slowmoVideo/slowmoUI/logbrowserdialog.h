#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>

class QTextBrowser;
class QPushButton;

class LogBrowserDialog : public QDialog
{
    Q_OBJECT
    
public:
    LogBrowserDialog(QWidget *parent = 0);
    ~LogBrowserDialog();
    
    public slots:
    void outputMessage( QtMsgType type, const QString &msg );
    
    protected slots:
    void save();
    
protected:
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void closeEvent( QCloseEvent *e );
    
    QTextBrowser *browser;
	  // static QTextEdit *m_LogEdit;
    QPushButton *clearButton;
    QPushButton *saveButton;
};

#endif // LOGDIALOG_H
