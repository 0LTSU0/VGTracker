#include "errordialog.h"
#include "ui_errordialog.h"

errorDialog::errorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::errorDialog)
{
    ui->setupUi(this);
}

errorDialog::~errorDialog()
{
    delete ui;
}

void errorDialog::setErrorMsg(int *errorLoc)
{
    QString errorMsg = "Invalid data in cell [" + QString::number(errorLoc[0]) + "," + QString::number(errorLoc[1]) + "]!";
    ui->errorMsg->setText(errorMsg);
}
