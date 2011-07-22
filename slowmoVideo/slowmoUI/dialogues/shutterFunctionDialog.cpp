#include "shutterFunctionDialog.h"
#include "ui_shutterFunctionDialog.h"

ShutterFunctionDialog::ShutterFunctionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShutterFunctionDialog)
{
    ui->setupUi(this);
}

ShutterFunctionDialog::~ShutterFunctionDialog()
{
    delete ui;
}
