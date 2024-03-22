#ifndef VGTRACKER_H
#define VGTRACKER_H

#include <QMainWindow>

#include "dbaccess.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class VGTracker;
}
QT_END_NAMESPACE

class VGTracker : public QMainWindow
{
    Q_OBJECT

public:
    VGTracker(QWidget *parent = nullptr);
    ~VGTracker();

private slots:
    void on_addPlatformButton_clicked();

    void on_addRow_clicked();

    void on_saveTable_clicked();

    void on_tableWidget_cellChanged(int row, int column);

private:
    Ui::VGTracker *ui;

    //db access class
    dbAccess dbaccess;
    QString visibleTable;
    std::vector<int> tableFormat;

    //table
    std::vector<tableRow> tableContent;
    std::vector<int> rowsWithChanges;

    // funtions
    bool addPlatformsToUI();
    std::vector<int> drawTable();
    void addCheckboxToTable(int row, int col, bool checked);
    void addEmptyRow(std::vector<int>* format);
    bool checkIfRowAlreadyChanged(int checkRow);
    bool validateTable();
    void saveNewRows();
    void applyChangesToVector();
};
#endif // VGTRACKER_H
