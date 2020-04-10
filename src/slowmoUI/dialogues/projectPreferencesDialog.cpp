#include "projectPreferencesDialog.h"
#include "ui_projectPreferencesDialog.h"

#include "lib/defs_sV.hpp"

ProjectPreferencesDialog::ProjectPreferencesDialog(ProjectPreferences_sV *prefs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectPreferencesDialog),
    m_projectPrefs(prefs)
{
    ui->setupUi(this);
    ui->canvas_xAxisFPS->setText(m_projectPrefs->canvas_xAxisFPS().toString());

    connect(ui->canvas_xAxisFPS, SIGNAL(textChanged(QString)), this, SLOT(slotCheckFPS()));
}
ProjectPreferencesDialog::~ProjectPreferencesDialog()
{
    delete ui;
}

void ProjectPreferencesDialog::accept()
{
    try {
        Fps_sV fps(ui->canvas_xAxisFPS->text());
        m_projectPrefs->canvas_xAxisFPS().num = fps.num;
        m_projectPrefs->canvas_xAxisFPS().den = fps.den;
    } catch (Error_sV &err) {}
    QDialog::accept();
}

void ProjectPreferencesDialog::slotCheckFPS()
{
    try {
        Fps_sV fps(ui->canvas_xAxisFPS->text());
        ui->canvas_xAxisFPS->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        fps.den = fps.num; // Just to hopefully avoid the fps being optimized out
    } catch (Error_sV &err) {
        ui->canvas_xAxisFPS->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
    }
}

