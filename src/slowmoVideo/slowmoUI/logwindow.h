class logwindow: public QMainWindow
{
        Q_OBJECT
 
public:
        logwindow( QWidget* parent = 0, const char* name = 0);
        ~logwindow();
 
        static void logMessage(QString aMsg);
};
