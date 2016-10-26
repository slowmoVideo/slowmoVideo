#include <QtCore/QDebug>
#include <QMainWindow>
#include <QWidget>
#include <QTextEdit>


class logwindow: public QMainWindow
{
        Q_OBJECT

			static logwindow *m_Me;
		  static QTextEdit *m_LogEdit;
 
public:
        logwindow( QWidget* parent = 0, const char* name = 0, Qt::WindowFlags flags = 0);
        ~logwindow();

			  static logwindow *getMe(); 
        static void logMessage(QString aMsg);
};
