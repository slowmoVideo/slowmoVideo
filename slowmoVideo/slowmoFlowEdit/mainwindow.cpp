#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "flowEditCanvas.h"

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#define MAX_SEARCH_SHIFT 100

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    canvas = new FlowEditCanvas(this);
    setCentralWidget(canvas);

    ui->actionQuit->setShortcut(QKeySequence("Ctrl+Q"));
    ui->actionOpen->setShortcut(QKeySequence("Ctrl+O"));
    ui->actionSave->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionPrev->setShortcut(QKeySequence("Ctrl+Left"));
    ui->actionNext->setShortcut(QKeySequence("Ctrl+Right"));

    bool b = true;
    b &= connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    b &= connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(slotOpenFlow()));
    b &= connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(slotSaveFlow()));
    b &= connect(ui->actionNext, SIGNAL(triggered()), this, SLOT(slotNextFile()));
    b &= connect(ui->actionPrev, SIGNAL(triggered()), this, SLOT(slotPrevFile()));
    Q_ASSERT(b);

    updateTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTitle()
{
    QString file = m_lastFlowFile;
    if (file.length() == 0) {
        file = "no file loaded";
    }
    setWindowTitle(QString("slowmo Flow Editor (%1)").arg(file));
}

QString MainWindow::nextFilename(QString originalName, int shift) const
{
    QStringList parts;
    QRegExp e("(\\d+)");
    int min = originalName.indexOf("_");
    int pos = 0;
    int prevPos = 0;
    while ((pos = e.indexIn(originalName, pos)) != -1) {
        parts << originalName.mid(prevPos, pos-prevPos);

        if (pos > min) {
            parts << QVariant(e.cap(1).toInt()+shift).toString();
        } else {
            parts << e.cap(1);
        }

        pos += e.matchedLength();
        prevPos = pos;
    }
    parts << originalName.mid(prevPos);
    return parts.join("");
}

void MainWindow::loadFlow(QString filename)
{
    canvas->slotLoadFlow(filename);
    m_lastFlowFile = filename;
    updateTitle();
}

void MainWindow::slotOpenFlow()
{
    QFileDialog dialog(this, "Open flow file");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Flow files (*.sVflow)");
    if (m_settings.value("directories/lastFlowDir", "").toString().length() > 0) {
        dialog.setDirectory(m_settings.value("directories/lastFlowDir", "").toString());
    }
    if (dialog.exec() == QDialog::Accepted) {
        m_settings.setValue("directories/lastFlowDir", QFileInfo(dialog.selectedFiles().at(0)).absolutePath());
        loadFlow(dialog.selectedFiles().at(0));
    }
}

void MainWindow::slotSaveFlow()
{
    statusBar()->showMessage("Saving ...", 3000);
    canvas->slotSaveFlow();
}

void MainWindow::slotNextFile()
{
    slotChangeFile(+1);
}

void MainWindow::slotPrevFile()
{
    slotChangeFile(-1);
}

void MainWindow::slotChangeFile(int shift)
{
    for (int i = 1; i < MAX_SEARCH_SHIFT; i++) {
        QString name = nextFilename(m_lastFlowFile, i*shift);
        if (QFileInfo(name).exists()) {
            loadFlow(name);
            return;
        }
    }
    QMessageBox::warning(this, "File not found", QString("The flow file %1 does not exist "
                                                         "(even searched %2 steps for a file in this direction).")
                         .arg(nextFilename(m_lastFlowFile, shift)).arg(MAX_SEARCH_SHIFT), QMessageBox::Ok);
}
