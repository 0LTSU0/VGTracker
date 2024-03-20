#ifndef WADDPLATFORM_H
#define WADDPLATFORM_H

#include <QDialog>

namespace Ui {
class WaddPlatform;
}

class WaddPlatform : public QDialog
{
    Q_OBJECT

public:
    explicit WaddPlatform(QWidget *parent = nullptr);
    ~WaddPlatform();

    QString getNewPlatformName();

private slots:
    void on_buttonBox_accepted();

    void on_newPlatformField2_textChanged(const QString &arg1);

private:
    Ui::WaddPlatform *ui;

    QString newPlatformName;
};

#endif // WADDPLATFORM_H
