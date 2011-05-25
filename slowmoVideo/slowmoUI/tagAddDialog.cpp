#include "tagAddDialog.h"
#include "ui_tagAddDialog.h"

TagAddDialog::TagAddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagAddDialog)
{
    ui->setupUi(this);
    ui->bOk->setEnabled(false);

    bool b = true;
    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    b &= connect(ui->tag, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
    b &= connect(ui->tag, SIGNAL(returnPressed()), ui->bOk, SLOT(click()));
    Q_ASSERT(b);
}

TagAddDialog::~TagAddDialog()
{
    delete ui;
}

void TagAddDialog::slotTextChanged(const QString &text)
{
    if (text.length() == 0) {
        ui->bOk->setEnabled(false);
    } else {
        ui->bOk->setEnabled(true);
        m_text = text;
    }
}
