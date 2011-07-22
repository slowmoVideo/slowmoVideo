#include "shutterFunctionDialog.h"
#include "ui_shutterFunctionDialog.h"
#include "project/shutterFunction_sV.h"
#include <QtGui/QPainter>

ShutterFunctionDialog::ShutterFunctionDialog(float dy, float t0, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShutterFunctionDialog),
    m_dy(dy),
    m_t0(t0)
{
    ui->setupUi(this);
    ui->lblcHeader->setText(ShutterFunction_sV::templateHeader);
    ui->lblcFooter->setText(ShutterFunction_sV::templateFooter);
    ui->function->clear();
    ui->function->insertPlainText(ShutterFunction_sV::templateBody);
    ui->shutterCurve->updateValues(dy, t0);

    bool b = true;
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->function, SIGNAL(textChanged()), this,  SLOT(slotUpateCurve()));
    Q_ASSERT(b);
}

ShutterFunctionDialog::~ShutterFunctionDialog()
{
    delete ui;
}

void ShutterFunctionDialog::slotUpateCurve()
{
    ui->shutterCurve->slotDisplayFunction(ui->function->toPlainText());
    ui->shutterCurve->repaint();
}

