#include "renderDialog.h"
#include "ui_renderDialog.h"

RenderDialog::RenderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenderDialog)
{
    ui->setupUi(this);
}

RenderDialog::~RenderDialog()
{
    delete ui;
}
