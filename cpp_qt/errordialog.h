#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>

namespace Ui {
class errorDialog;
}

class errorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit errorDialog(QWidget *parent = nullptr);
    ~errorDialog();

    void setErrorMsg(int *errorLoc);
    void setErrorMsg(QString msg);

private:
    Ui::errorDialog *ui;
};

#endif // ERRORDIALOG_H
