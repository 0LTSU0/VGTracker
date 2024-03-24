#ifndef DBACCESS_H
#define DBACCESS_H

#include <sqlite3.h>
#include <QString>
#include "enums.h"

class dbAccess
{
private:
    sqlite3 *db;

    std::string tableFormat = "(id INTEGER PRIMARY KEY,"
                              "name TEXT NOT NULL,"
                              "completed BOOLEAN,"
                              "platinum BOOLEAN,"
                              "pricePaid REAL,"
                              "notes TEXT);";

public:
    dbAccess();
    int loadDatabase();
    void closeDatabase();
    void getPlatforms(std::vector<QString>* outputPlatforms);
    int addNewPlatfromToDb(QString *tableName);
    void getTableColumns(QString* tableName, std::vector<QString>* result, std::vector<int>* datatypes);
    void getEntriesForPlatform(QString *visibleTable, std::vector<tableRow> *tableContent);
    void addNewRowToDB(tableRow*, QString*);
    void editRowInDb(tableRow*, QString*);
};

#endif // DBACCESS_H
