#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"

#include "project/flowSourceV3D_sV.h"
#include "lib/defs_sV.hpp"
#include "lib/avconvInfo_sV.h"
#include "../../project/videoFrameSource_sV.h"
#include <QtCore/QProcess>
#include <QFileDialog>

//TODO: header ?
int isOCLsupported();
QList<QString> oclFillDevices(void);

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->buildFlow->setText(m_settings.value("binaries/v3dFlowBuilder", "").toString());
    ui->ffmpeg->setText(m_settings.value("binaries/ffmpeg", "ffmpeg").toString());
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#if QT_VERSION >= 0x040700
    ui->buildFlow->setPlaceholderText(QApplication::translate("PreferencesDialog", "flowBuilder binary location", 0, QApplication::UnicodeUTF8));
#endif
#else
    ui->buildFlow->setPlaceholderText(QApplication::translate("PreferencesDialog", "flowBuilder binary location", 0));
#endif

// TODO: qcombox box instead ?
    m_flowMethodGroup.addButton(ui->methodOCV,-1);
    m_flowMethodGroup.addButton(ui->methodV3D,-1);
    m_flowMethodGroup.addButton(ui->methodOCL,-1);
    m_flowMethodGroup.setExclusive(true);

    QString method = m_settings.value("preferences/flowMethod", "OCL").toString();
    qDebug() << "method is : " << method;
    int ocl_support = isOCLsupported();
    if (ocl_support)  {
    	ui->methodOCL->setEnabled(true);
    	// add OpenCL devices
    	QList<QString> ocldevices = oclFillDevices();
        ui->ocl_device->addItems(ocldevices);    
    }
    else
    	ui->methodOCL->setEnabled(false);
    if ("V3D" == method) {
        ui->methodV3D->setChecked(true);
    } 
    if ("OCL" == method) {
        ui->methodOCL->setChecked(true);
        //qDebug() << "setting OCL";
        // restore selected device for OpenCL
        int dev = m_settings.value("preferences/oclDriver", 0).toInt();
        ui->ocl_device->setCurrentIndex(dev);
    } else {
        	ui->methodOCV->setChecked(true);
        // TODO: for testing
        ui->oclAlgo->clear();
        ui->oclAlgo->addItem(tr("Farnback"),QVariant(1));
        ui->oclAlgo->addItem(tr("Dual TVL1"),QVariant(2));
        int algo = m_settings.value("preferences/oclAlgo", 0).toInt();
        ui->oclAlgo->setCurrentIndex(algo);
    }
	
    // state of threading
    bool precalc = m_settings.value("preferences/precalcFlow", true).toBool();
    if (precalc)
        ui->precalcFlow->setChecked(true);
    else
        ui->precalcFlow->setChecked(false);
    
    connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->bCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->bBuildFlow, SIGNAL(clicked()), this, SLOT(slotBrowseFlow()));
    connect(ui->buildFlow, SIGNAL(textChanged(QString)), this, SLOT(slotValidateFlowBinary()));
    connect(&m_flowMethodGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotUpdateFlowMethod()));
    connect(ui->bFFmpeg, SIGNAL(clicked()), this, SLOT(slotBrowseFfmpeg()));

    if (!FlowSourceV3D_sV::validateFlowBinary(ui->buildFlow->text())) {
        FlowSourceV3D_sV::correctFlowBinaryLocation();
        ui->buildFlow->setText(m_settings.value("binaries/v3dFlowBuilder", "").toString());
    }
    slotValidateFlowBinary();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::accept()
{
    // V3D binary location
    if (FlowSourceV3D_sV::validateFlowBinary(ui->buildFlow->text())) {
        m_settings.setValue("binaries/v3dFlowBuilder", ui->buildFlow->text());
    }
    
    // Flow method
    QString method("OpenCV-Farnback");
    if (ui->methodV3D->isChecked()) {
        method = "V3D";
    }
    if (ui->methodOCL->isChecked()) {
        method = "OCL";
        //TODO: maybe need to use text version ?
        int dev = ui->ocl_device->currentIndex();
        qDebug() << "driver choosen is : " << dev;
        m_settings.setValue("preferences/oclDriver", dev);
    }
    int algo = ui->oclAlgo->currentIndex();
    m_settings.setValue("preferences/oclAlgo",algo);
    
    qDebug() << "saving method  : " << method;
    m_settings.setValue("preferences/flowMethod", method);
    
    // thread calc
    bool precalc = true;
    
    
    if (ui->precalcFlow->isChecked()) {
        precalc = true;
    } else
        precalc = false;
    m_settings.setValue("preferences/precalcFlow", precalc
                        );
    
    // ffmpeg location
    if (AvconvInfo::testAvconvExecutable(ui->ffmpeg->text())) {
        m_settings.setValue("binaries/ffmpeg", ui->ffmpeg->text());
    } else {
        qDebug() << "Not a valid ffmpeg/avconv executable: " << ui->ffmpeg->text();
    }
    
    // Store the values right now
    m_settings.sync();
    
    QDialog::accept();
}

void PreferencesDialog::slotUpdateFlowMethod()
{
}

void PreferencesDialog::slotUpdateFfmpeg()
{
    m_settings.setValue("binaries/ffmpeg", ui->ffmpeg->text());
}

void PreferencesDialog::slotValidateFlowBinary()
{
    if (FlowSourceV3D_sV::validateFlowBinary(ui->buildFlow->text())) {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        ui->methodV3D->setEnabled(true);
    } else {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        ui->methodV3D->setEnabled(false);
        //ui->methodOCV->setChecked(true);
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

void PreferencesDialog::slotBrowseFfmpeg()
{
    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setDirectory(QFileInfo(ui->ffmpeg->text()).absolutePath());
    if (dialog.exec() == QDialog::Accepted) {
        ui->ffmpeg->setText(dialog.selectedFiles().at(0));
        //slotValidateFffmpegBinary();
    }
}
