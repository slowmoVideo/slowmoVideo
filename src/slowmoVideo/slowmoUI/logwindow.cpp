// ?
#include "logwindow.h"

QTextEdit *logwindow::m_LogEdit = 0;
logwindow *logwindow::m_Me = 0;
 
 
void myMessageOutput( QtMsgType type, const char *msg )
{
        QString sMsg = msg;
        if(sMsg.isEmpty()) return;
        if(sMsg.length() >= 8195) sMsg.truncate(8195);

        logwindow *mw = logwindow::getMe();
	if (mw) 
		
        if(mw && mw->logToFile())
                mw->logMessageToFile(msg);
}

logwindow::logwindow( QWidget* parent, const char* name  , Qt::WindowFlags flags  )
: QMainWindow( parent, name ,flags)
{
        if(!m_Me) m_Me = this;

        m_LogEdit = new QTextEdit(this);
        m_LogEdit->setTextFormat(Qt::LogText);
 
}
