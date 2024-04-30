#ifndef VGTRACKER_H
#define VGTRACKER_H

#include <QMainWindow>

#include "dbaccess.h"
#include "configurator.h"
#include "qlabel.h"
#include "qprogressbar.h"

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
    std::wstring databasePath = L"";

    //table
    std::vector<tableRow> tableContent; //The order of this SHOULD always correspond to order of rows in the table
    std::vector<int> rowsWithChanges;
    QString visibleTable;
    std::vector<int> tableFormat;
    bool ignoreTableChanges = false;
    int currentlyActiveRow = -1;

    //stats
    statValues statVals;

    // funtions
    bool addPlatformsToUI();
    std::vector<int> drawTable();
    void addPreExistingEntriesToTable();
    void addCheckboxToTable(int row, int col, bool checked);
    void addEmptyRow(std::vector<int>* format);
    bool checkIfRowAlreadyChanged(int checkRow);
    bool validateTable(int *errorLoc);
    int saveNewRows();
    void saveEditedRows();
    void deleteDeletedRows();
    void applyChangesToVector();
    void clearAndRedrawTable();
    void setDbFile(std::wstring);
    QProgressBar *progressBar;
    QLabel *statLabel;
    void initStatusBar();
    void updateStatusMessage(QString);
    void updateStatLabel(int, float, int, int);
    float recalculateTotalPrice();

public:
    void initUI();
};
#endif // VGTRACKER_H
