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

    void on_platformSelectDropdown_currentTextChanged(const QString &arg1);

    void on_pushButton_clicked();

    void on_tableWidget_cellPressed(int row, int column);

private:
    Ui::VGTracker *ui;

    //db access class
    dbAccess dbaccess;

    //table
    std::vector<tableRow> tableContent;
    std::vector<int> rowsWithChanges;
    std::vector<int> markedForDelete;
    QString visibleTable;
    std::vector<int> tableFormat;
    bool ignoreTableChanges = false;
    int currentlyActiveRow = -1;

    // funtions
    bool addPlatformsToUI();
    std::vector<int> drawTable();
    void addPreExistingEntriesToTable();
    void addCheckboxToTable(int row, int col, bool checked);
    void addEmptyRow(std::vector<int>* format);
    bool checkIfRowAlreadyChanged(int checkRow);
    bool validateTable(int *errorLoc);
    void saveNewRows();
    void saveEditedRows();
    void applyChangesToVector();
    void clearAndRedrawTable();
};
#endif // VGTRACKER_H
