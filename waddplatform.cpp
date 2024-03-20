#include "waddplatform.h"
#include "qpushbutton.h"
#include "ui_waddplatform.h"

WaddPlatform::WaddPlatform(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WaddPlatform)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

WaddPlatform::~WaddPlatform()
{
    delete ui;
}

QString WaddPlatform::getNewPlatformName()
{
    return newPlatformName;
}

void WaddPlatform::on_buttonBox_accepted()
{
    newPlatformName = ui->newPlatformField2->text();
}

void WaddPlatform::on_newPlatformField2_textChanged(const QString &arg1)
{
    if (!ui->newPlatformField2->text().isEmpty())
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

