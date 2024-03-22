#include "vgtracker.h"
#include "./ui_vgtracker.h"
#include "qlineedit.h"
#include "waddplatform.h"
//#include "enums.h"
#include <QCheckBox>
#include <QDebug>

#include <iostream>

VGTracker::VGTracker(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VGTracker)
{
    ui->setupUi(this);
    dbaccess.loadDatabase(); //init db access
    bool tablesExists = this->addPlatformsToUI(); //add pre-existing platforms to dropdown
    if (tablesExists)
    {
        tableFormat = this->drawTable(); //addPlatformsToUI() sets visibleTable to something. This takes column names for that table in the db and adds them to the header
        this->addEmptyRow(&tableFormat); //temptest, add empty row to table
    }
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
    ui->tableWidget->setCellWidget(row,col,checkBoxWidget);
}

// Creates an empty row to table. Input param is a vector of sql datatypes
void VGTracker::addEmptyRow(std::vector<int>* format)
{
    int currentNumRows = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(currentNumRows + 1);
    int columnCounter = 0;
    for (const auto &dtype : *format)
    {
        switch(dtype){
        case types::columnType::integer:
            if (columnCounter == 0)
            {
                //first column is integer -> should be sql id -> disable editing of the column
                QTableWidgetItem* item = new QTableWidgetItem("*"); //id would go here but use * as undefined
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                ui->tableWidget->setItem(currentNumRows, columnCounter, item);
            }
            break;
        case types::columnType::text:
            break; //default behavior seems fine
        case types::columnType::boolean:
            this->addCheckboxToTable(currentNumRows, columnCounter, false);
            break;
        case types::columnType::decimal:
            break; //TODO: try to find a way to limit the characters that can be inputted
        }
        columnCounter++;
    }
    tableContent.push_back(tableRow());
    tableContent.back().id = notYetInDb;
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
        std::cout << "Adding new platform with input " << newPlatform.toStdString() << std::endl;
        res = dbaccess.addNewPlatfromToDb(&newPlatform);
        if (!res)
        {
            ui->platformSelectDropdown->addItem(newPlatform);
        }
    }
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

bool VGTracker::validateTable()
{
    //TODO: Check if all data inputted to the table are valid and can be saved to db
    return true;
}

void VGTracker::saveNewRows()
{
    for (auto &tableRow : tableContent)
    {
        if(tableRow.id == notYetInDb)
        {
            dbaccess.addNewRowToDB(&tableRow, &visibleTable);
        }
    }
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

// Handler for clicking "new row" button in main gui
void VGTracker::on_addRow_clicked()
{
    this->addEmptyRow(&tableFormat);
}

// Handler for clicking button that saves changes done to the table to database
void VGTracker::on_saveTable_clicked()
{
    if (VGTracker::validateTable())
    {
        VGTracker::applyChangesToVector();
        VGTracker::saveNewRows();
        //saveEditedRows() //TODO. also remember to check that only previously saved rows are stored
    }
}

// This should get called whenever data in some cell changes
void VGTracker::on_tableWidget_cellChanged(int row, int column)
{
    if (!column && tableFormat.at(0) == types::columnType::integer)
    {
        return; //id is not user changable -> ignore
    }

    qDebug() << "data changed: " << row << " " << column;

    //temptest
    //if (tableFormat.at(column) == types::columnType::boolean)
    //{
    //    // this is fucking stupid but its how the cells are constructed ¯\_(ツ)_/¯
    //    QWidget *cellWidget = ui->tableWidget->cellWidget(row, column);
    //    QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(cellWidget->layout());
    //    QCheckBox *checkBox = qobject_cast<QCheckBox*>(layout->itemAt(0)->widget());
    //    bool isChecked = checkBox->isChecked();
    //    qDebug() << isChecked;
    //}

    if (!VGTracker::checkIfRowAlreadyChanged(row))
    {
        rowsWithChanges.push_back(row);
    }
}

