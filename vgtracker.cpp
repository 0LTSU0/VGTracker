#include "vgtracker.h"
#include "./ui_vgtracker.h"
#include "waddplatform.h"
#include "errordialog.h"
//#include "enums.h"
#include <QCheckBox>
#include <QDebug>

#include <iostream>
#include <algorithm>

VGTracker::VGTracker(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VGTracker)
{
    ui->setupUi(this);
    dbaccess.loadDatabase(); //init db access
    ignoreTableChanges = true; //TableWidget will fire bunch of row changed calls when table is being drawn -> use this to ignore while table is being constructed
    bool tablesExists = this->addPlatformsToUI(); //add pre-existing platforms to dropdown
    if (tablesExists)
    {
        tableFormat = this->drawTable(); //addPlatformsToUI() sets visibleTable to something. This takes column names for that table in the db and adds them to the header
        this->addPreExistingEntriesToTable(); //get items from db for the currently visible table and add them to the tablewidget
        //this->addEmptyRow(&tableFormat); //temptest, add empty row to table
    }
    ignoreTableChanges = false;
}

VGTracker::~VGTracker()
{
    dbaccess.closeDatabase();
    delete ui;
}

// Asks for platforms that are present in database and adds them to the platform select dropdown
// Returns true if some platforms existed and were added to dropdown, otherwise false
bool VGTracker::addPlatformsToUI()
{
    std::vector<QString> platforms;
    dbaccess.getPlatforms(&platforms);
    bool tablesExists = false;
    for (int i = 0; i < platforms.size(); i++)
    {
        ui->platformSelectDropdown->addItem(platforms.at(i));
        if (!tablesExists) // Set visible table to whatever was added first so that they match. TODO: make it remember what platform was viewed last and use that instead when possible
        {
            visibleTable = platforms.at(i);
        }
        tablesExists = true;
    }
    return tablesExists;
}

// "creates" a table and sets headers to keys in database for currently visible table
// returns a vector of datatypes corresponding to columns
std::vector<int> VGTracker::drawTable()
{
    std::vector<QString> tableColumns;
    std::vector<int> tableColumnDataTypes;
    dbaccess.getTableColumns(&visibleTable, &tableColumns, &tableColumnDataTypes);
    //ui->tableWidget->setRowCount(2);
    ui->tableWidget->setColumnCount(tableColumns.size());
    ui->tableWidget->setStyleSheet("QHeaderView::section { background-color:#d9d9d9; font-weight:bold }"); //header to light grey and text bold
    for (int i = 0; i < tableColumns.size(); i++)
    {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(tableColumns.at(i)));
        if (tableColumnDataTypes.at(i) == types::columnType::text)
        {
            ui->tableWidget->setColumnWidth(i, ui->tableWidget->columnWidth(i) * 3);
        }
        else if (tableColumnDataTypes.at(i) == types::columnType::integer && !i) //if first column is int, assume it's id -> make column small
        {
            ui->tableWidget->setColumnWidth(i, static_cast<int>(ui->tableWidget->columnWidth(i) * 0.5));
        }
        qDebug() << tableColumnDataTypes.at(i);
    }

    return tableColumnDataTypes;
}

// get entries for the visible platform from db and add to display
void VGTracker::addPreExistingEntriesToTable()
{
    dbaccess.getEntriesForPlatform(&visibleTable, &tableContent);
    for (auto &entry : tableContent)
    {
        //TODO make less hardcoded
        int currentNumRows = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(currentNumRows + 1);
        QTableWidgetItem* item = new QTableWidgetItem(QString::number(entry.id)); //id
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(currentNumRows, 0, item);
        item = new QTableWidgetItem(entry.name); //name
        ui->tableWidget->setItem(currentNumRows, 1, item);
        this->addCheckboxToTable(currentNumRows, 2, entry.completed); //completed
        this->addCheckboxToTable(currentNumRows, 3, entry.platinum); //platinum
        item = new QTableWidgetItem(QString::number(entry.pricePaid)); //price paid
        ui->tableWidget->setItem(currentNumRows, 4, item);
        item = new QTableWidgetItem(entry.notes); //notes
        ui->tableWidget->setItem(currentNumRows, 5, item);
    }
}

// helper function for creating a checkbox into a cell specified by row/column
void VGTracker::addCheckboxToTable(int row, int col, bool checked=false)
{
    //stolen from https://stackoverflow.com/a/59374678
    QWidget *checkBoxWidget = new QWidget();
    QCheckBox *checkBox = new QCheckBox();      // We declare and initialize the checkbox
    QHBoxLayout *layoutCheckBox = new QHBoxLayout(checkBoxWidget); // create a layer with reference to the widget
    layoutCheckBox->addWidget(checkBox);            // Set the checkbox in the layer
    layoutCheckBox->setAlignment(Qt::AlignCenter);  // Center the checkbox
    layoutCheckBox->setContentsMargins(0,0,0,0);    // Set the zero padding
    connect(checkBox, &QCheckBox::clicked, this, [=]() {
        emit ui->tableWidget->cellChanged(row, col); // Emit cellChanged signal when checkbox is clicked
    });
    checkBox->setChecked(checked);
    ui->tableWidget->setCellWidget(row,col,checkBoxWidget);
}

// Creates an empty row to table. Input param is a vector of sql datatypes
void VGTracker::addEmptyRow(std::vector<int>* format)
{
    int currentNumRows = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(currentNumRows + 1);
    int columnCounter = 0;
    QTableWidgetItem* item;
    for (const auto &dtype : *format)
    {
        switch(dtype){
        case types::columnType::integer:
            if (columnCounter == 0)
            {
                //first column is integer -> should be sql id -> disable editing of the column
                item = new QTableWidgetItem("*"); //id would go here but use * as undefined
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                ui->tableWidget->setItem(currentNumRows, columnCounter, item);
            }
            break;
        case types::columnType::text:
            item = new QTableWidgetItem();
            ui->tableWidget->setItem(currentNumRows, columnCounter, item);
            break; //default behavior seems fine
        case types::columnType::boolean:
            this->addCheckboxToTable(currentNumRows, columnCounter, false);
            break;
        case types::columnType::decimal:
            item = new QTableWidgetItem("");
            ui->tableWidget->setItem(currentNumRows, columnCounter, item);
            break; //TODO: try to find a way to limit the characters that can be inputted
        }
        columnCounter++;
    }
    tableContent.push_back(tableRow());
    tableContent.back().id = notYetInDb;
}

// Check if it's already known that row being edited differs from database
bool VGTracker::checkIfRowAlreadyChanged(int checkRow)
{
    bool wasAlreadyChanged = false;
    for (auto &row : rowsWithChanges)
    {
        if (row == checkRow)
        {
            wasAlreadyChanged = true;
            break;
        }
    }
    return wasAlreadyChanged;
}

//Check if all data in the table is valid
bool VGTracker::validateTable(int *errorLoc)
{
    QTableWidgetItem* item;
    bool ok;
    //TODO: Return some kind of error message that explains why the cell is invalid
    //also should theoritically be enough to loop changed rows?
    for (int row=0; row < ui->tableWidget->rowCount(); row++)
    {
        //if row is marked for delete, it does not matter what data it has
        if (tableContent.at(row)._isMarkedForDelete)
        {
            continue;
        }

        for (int col=0; col < ui->tableWidget->columnCount(); col++)
        {
            switch(tableFormat.at(col)){
            case types::columnType::integer:
                if (col == 0) // don't bother with primary key
                {
                    continue;
                }
                item = ui->tableWidget->item(row, col);
                if (item != nullptr)
                {
                    item->text().toInt(&ok);
                    if (!ok)
                    {
                        qDebug() << "int check failed in " << row << " " << col;
                        errorLoc[0] = row;
                        errorLoc[1] = col;
                        return false;
                    }
                }
                break;
            case types::columnType::text:
                if (col == 1) //currently name is required in sql which happens to be column 1. TODO: somehow flag which columns are notnull in sql
                {
                    item = ui->tableWidget->item(row, col);
                    if (item == nullptr) // if nullptr, then cell is definately empty -> return false
                    {
                        qDebug() << "text check failed in " << row << " " << col;
                        errorLoc[0] = row;
                        errorLoc[1] = col;
                        return false;
                    }
                    else // if not nullptr but text inside cell is empty -> return false
                    {
                        if (item->text().isEmpty())
                        {
                            qDebug() << "text check failed in " << row << " " << col << " empty";
                            errorLoc[0] = row;
                            errorLoc[1] = col;
                            return false;
                        }
                    }

                }
                break; //text can be anything usually -> no need to check
            case types::columnType::boolean:
                break; //assuming table was constructed correctly, there should be a checkbox that is either checked or not -> no need to check anything
            case types::columnType::decimal:
                item = ui->tableWidget->item(row, col);
                if (item != nullptr)
                {
                    if (!item->text().isEmpty())
                    {
                        item->text().toFloat(&ok);
                        if (!ok)
                        {
                            qDebug() << "float check failed in " << row << " " << col;
                            errorLoc[0] = row;
                            errorLoc[1] = col;
                            return false;
                        }
                    }
                }
                break;
            }
        }

    }
    return true;
}

void VGTracker::saveNewRows()
{
    for (auto &tableRow : tableContent)
    {
        if(tableRow.id == notYetInDb)
        {
            // Only send the new line to db if it's not marked for deletion
            if (!tableRow._isMarkedForDelete)
            {
                dbaccess.addNewRowToDB(&tableRow, &visibleTable);
            }
        }
    }
}

void VGTracker::saveEditedRows()
{
    // TODO: this is stupid and slow, could maybe instead just do "for row in rowsWithChanges"
    for (int tableItemRow = 0; tableItemRow < tableContent.size(); tableItemRow++)
    {
        // https://stackoverflow.com/a/3450906
        if (std::find(rowsWithChanges.begin(), rowsWithChanges.end(), tableItemRow) != rowsWithChanges.end())
        {
            dbaccess.editRowInDb(&tableContent.at(tableItemRow), &visibleTable);
        }
    }
}

void VGTracker::deleteDeletedRows()
{
    for (auto &row : tableContent)
    {
        if (row._isMarkedForDelete && row.id != notYetInDb)
        {
            dbaccess.deleteRowWithId(&row.id, &visibleTable);
        }
    }
}

void VGTracker::clearAndRedrawTable()
{
    ignoreTableChanges = true;
    rowsWithChanges.clear();
    tableContent.clear();
    tableFormat.clear();
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    tableFormat = this->drawTable(); //visibleTable should have been set to correct value before this function is called
    this->addPreExistingEntriesToTable(); //get items from db for the visibleTable and add them to the tablewidget
    ignoreTableChanges = false;
}

bool _qStringToFloat(QString str, float* res)
{
    bool ok = false;
    *res = str.toFloat(&ok);
    return ok;
}

// Go over edited rows and add their data to corresponding entries in tableContent vector
void VGTracker::applyChangesToVector()
{
    for (auto &row : rowsWithChanges)
    {
        tableRow &tableEntry = tableContent.at(row);
        if (tableEntry._isMarkedForDelete)//if edited row is also marked for delete, don't bother with changing data in the vector
        {
            continue;
        }

        // TODO make this less "hardcoded". Format of table is known but currently cannot tell what column correspond to what member in tableEntry
        QTableWidgetItem* item = ui->tableWidget->item(row, 1);
        if (item != nullptr)
        {
            tableEntry.name = item->text();
        }
        QWidget *cellWidget = ui->tableWidget->cellWidget(row, 2);
        QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(cellWidget->layout());
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(layout->itemAt(0)->widget());
        if (checkBox != nullptr)
        {
            tableEntry.completed = checkBox->isChecked();
        }
        cellWidget = ui->tableWidget->cellWidget(row, 3);
        layout = qobject_cast<QHBoxLayout*>(cellWidget->layout());
        checkBox = qobject_cast<QCheckBox*>(layout->itemAt(0)->widget());
        if (checkBox != nullptr)
        {
            tableEntry.platinum = checkBox->isChecked();
        }
        item = ui->tableWidget->item(row, 4);
        if (item != nullptr)
        {
            float price;
            if (_qStringToFloat(item->text(), &price))
            {
                tableEntry.pricePaid = price;
            }
        }
        item = ui->tableWidget->item(row, 5);
        if (item != nullptr)
        {
            tableEntry.notes = item->text();
        }
    }
}

// Handler for clicking "add platform" button in main gui
void VGTracker::on_addPlatformButton_clicked()
{
    WaddPlatform addPlatformDialog;
    addPlatformDialog.setModal(true); //blocks using main window
    int res = addPlatformDialog.exec();
    if (res == QDialog::Accepted)
    {
        QString newPlatform = addPlatformDialog.getNewPlatformName();
        qDebug() << "Adding new platform with input " << newPlatform;
        visibleTable = newPlatform;
        res = dbaccess.addNewPlatfromToDb(&newPlatform);
        if (!res)
        {
            ui->platformSelectDropdown->addItem(newPlatform);
            qDebug() << "on_addPlatformButton_clicked() changed visibleTable to " << visibleTable;
            ui->platformSelectDropdown->setCurrentIndex(ui->platformSelectDropdown->count() - 1);
            clearAndRedrawTable(); //reset table and draw with new platform
        }
    }
}

// Handler for clicking "new row" button in main gui
void VGTracker::on_addRow_clicked()
{
    this->addEmptyRow(&tableFormat);
}

// Handler for clicking button that saves changes done to the table to database
void VGTracker::on_saveTable_clicked()
{
    int errorLocation[2];
    if (VGTracker::validateTable(errorLocation))
    {
        VGTracker::applyChangesToVector();
        VGTracker::saveNewRows();
        VGTracker::saveEditedRows();
        VGTracker::deleteDeletedRows();
        VGTracker::clearAndRedrawTable();
    }
    else
    {
        // If table validation failed, show error message
        errorDialog errordialog;
        errordialog.setModal(true); //blocks using main window
        errordialog.setErrorMsg(errorLocation);
        int res = errordialog.exec();
        qDebug() << "error dialog closed " << res;
    }
}

// This should get called whenever data in some cell changes
void VGTracker::on_tableWidget_cellChanged(int row, int column)
{
    if ((!column && tableFormat.at(0) == types::columnType::integer) || ignoreTableChanges)
    {
        return; //id is not user changable -> ignore. Also ignore everything if ignoreTableChanges flag is set
    }

    qDebug() << "data changed: " << row << " " << column;
    if (!VGTracker::checkIfRowAlreadyChanged(row))
    {
        rowsWithChanges.push_back(row);
    }
}

// This is called whenever the platform select dropdown is changed
void VGTracker::on_platformSelectDropdown_currentTextChanged(const QString &arg1)
{
    if (ignoreTableChanges)
    {
        return;
    }
    visibleTable = arg1;
    clearAndRedrawTable();
}

// Callback for "mark for delete" button
// If row has not been marked for delete yet, make it red and add to markedForDelete vector
// If row is in markedForDelte vector, remove it from it and undo the red color
void VGTracker::on_pushButton_clicked()
{
    ignoreTableChanges = true; //changing colors of cells will fire some of these
    if (currentlyActiveRow == -1) //"nothing selected" value should be -1
    {
        return;
    }

    bool rowMarkedForDelete = tableContent.at(currentlyActiveRow)._isMarkedForDelete;

    for (int i = 0; i<tableFormat.size(); i++)
    {
        if (tableFormat.at(i) == types::columnType::boolean) // normal way of setting background color does not work on boolean fields
        {
            QWidget *cellWidget = ui->tableWidget->cellWidget(currentlyActiveRow, i);
            if (cellWidget == nullptr)
            {
                qDebug() << "cellWidget was nullptr in on_pushButton_clicked! selectedRow: " << currentlyActiveRow;
                continue; //should not be possible but just in case
            }
            if (rowMarkedForDelete)
            {
                cellWidget->setStyleSheet("background-color: white;");
            }
            else
            {
                cellWidget->setStyleSheet("background-color: red;");
            }
        }
        else
        {

            if (rowMarkedForDelete)
            {
                ui->tableWidget->item(currentlyActiveRow,i)->setBackground(QColor(255,255,255));
            }
            else
            {
                ui->tableWidget->item(currentlyActiveRow,i)->setBackground(QColor(255,0,0));
            }
        }
    }

    tableContent.at(currentlyActiveRow)._isMarkedForDelete = !rowMarkedForDelete;
    ignoreTableChanges = false;
}

// This gets called whenever some cell in table is pressed
// Updates VGTracker.currentlyActiveRow.
// TODO: clicking the row number in the table doesn't call this even though it probably should
void VGTracker::on_tableWidget_cellPressed(int row, int column)
{
    currentlyActiveRow = row;
}

