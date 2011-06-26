#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"

#include "../lib/defs_sV.hpp"
#include <QtCore/QProcess>
#include <QtGui/QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->buildFlow->setText(m_settings.value("binaries/v3dFlowBuilder", "").toString());
    ui->lambda->setValue(m_settings.value("settings/v3dFlowBuilder/lambda", "5.0").toDouble());

    bool b = true;
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    b &= connect(ui->bCancel, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bBuildFlow, SIGNAL(clicked()), this, SLOT(slotBrowseFlow()));
    b &= connect(ui->buildFlow, SIGNAL(textChanged(QString)), this, SLOT(slotValidateFlowBinary()));
    Q_ASSERT(b);

    slotValidateFlowBinary();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::accept()
{
    if (slotValidateFlowBinary()) {
        m_settings.setValue("binaries/v3dFlowBuilder", ui->buildFlow->text());
    }
    m_settings.setValue("settings/v3dFlowBuilder/lambda", ui->lambda->value());
    QDialog::accept();
}

bool PreferencesDialog::slotValidateFlowBinary()
{
    bool valid = false;
    if (QFile(ui->buildFlow->text()).exists() && QFileInfo(ui->buildFlow->text()).isExecutable()) {
        QProcess process;
        QStringList args;
        args << "--identify";
        process.start(ui->buildFlow->text(), args);
        process.waitForFinished(1000);
        QString output(process.readAllStandardOutput());
        if (output.startsWith("flowBuilder")) {
            valid = true;
        } else {
            qDebug() << "Invalid output from flow executable: " << output;
        }
        process.terminate();
    }
    if (valid) {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
    } else {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
    }
    return valid;
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
