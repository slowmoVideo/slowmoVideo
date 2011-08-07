#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"

#include "lib/defs_sV.hpp"
#include <QtCore/QProcess>
#include <QtGui/QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->buildFlow->setText(m_settings.value("binaries/v3dFlowBuilder", "").toString());

    bool b = true;
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    b &= connect(ui->bCancel, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bBuildFlow, SIGNAL(clicked()), this, SLOT(slotBrowseFlow()));
    b &= connect(ui->buildFlow, SIGNAL(textChanged(QString)), this, SLOT(slotValidateFlowBinary()));
    Q_ASSERT(b);

    if (!validateFlowBinary(ui->buildFlow->text())) {
        trySetFlowBinary();
        if (validateFlowBinary(ui->buildFlow->text())) {
            m_settings.setValue("binaries/v3dFlowBuilder", ui->buildFlow->text());
        }
    }
    slotValidateFlowBinary();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::accept()
{
    if (validateFlowBinary(ui->buildFlow->text())) {
        m_settings.setValue("binaries/v3dFlowBuilder", ui->buildFlow->text());
    }
    QDialog::accept();
}

void PreferencesDialog::trySetFlowBinary()
{
    QStringList paths;
    paths << ui->buildFlow->text();
    paths << QDir::currentPath() + "/flowBuilder";
    paths << "/usr/bin/flowBuilder" << "/usr/local/bin/flowBuilder";
    for (int i = 0; i < paths.size(); i++) {
        if (validateFlowBinary(paths.at(i))) {
            ui->buildFlow->setText(paths.at(i));
            break;
        }
    }
}

bool PreferencesDialog::validateFlowBinary(const QString path) const
{
    bool valid = false;
    qDebug() << "Checking " << path << " ...";
    if (QFile(path).exists() && QFileInfo(path).isExecutable()) {
        QProcess process;
        QStringList args;
        args << "--identify";
        process.start(path, args);
        process.waitForFinished(2000);
        QString output(process.readAllStandardOutput());
        if (output.startsWith("flowBuilder")) {
            valid = true;
            qDebug() << path << " is valid.";
        } else {
            qDebug() << "Invalid output from flow executable: " << output;
        }
        process.terminate();
    }
    return valid;
}

void PreferencesDialog::slotValidateFlowBinary()
{
    if (validateFlowBinary(ui->buildFlow->text())) {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
    } else {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
    }
}

void PreferencesDialog::slotBrowseFlow()
{
    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setDirectory(QFileInfo(ui->buildFlow->text()).absolutePath());
    if (dialog.exec() == QDialog::Accepted) {
        ui->buildFlow->setText(dialog.selectedFiles().at(0));
        slotValidateFlowBinary();
    }
}
