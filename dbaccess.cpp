#include "dbaccess.h"
#include <iostream>
#include <QDebug>

#include <string>

dbAccess::dbAccess() {}

// Load database. If database does not exist, it should be created
int dbAccess::loadDatabase()
{
    int res = sqlite3_open("database.db", &db);
    std::cout << "loadDatabase() " << res << std::endl;
    return res;
}

// Close database. This should be called when gui is closed
void dbAccess::closeDatabase()
{
    std::cout << "closeDatabase()" << std::endl;
    sqlite3_close(db);
}

// Get platforms (i.e. names of tables) that have already been saved to database
void dbAccess::getPlatforms(std::vector<QString>* outputPlatforms)
{
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table'";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *tableName = sqlite3_column_text(stmt, 0);
        outputPlatforms->push_back(QString::fromUtf8(reinterpret_cast<const char*>(tableName)));
    }

    sqlite3_finalize(stmt);
}

// Add new platform into the database as a table
int dbAccess::addNewPlatfromToDb(QString * tableName)
{
    char *zErrMsg = 0;
    std::string sqlStatement = "CREATE TABLE IF NOT EXISTS " +
                               tableName->toStdString() +
                               tableFormat;
    const char* sql = sqlStatement.c_str();
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErrMsg);
    std::cout << "addNewPlatfromToDb " << rc << std::endl;
    return rc;
}

// Gets column names for table passed in as tableName
void dbAccess::getTableColumns(QString* tableName, std::vector<QString>* result, std::vector<int>* datatypes)
{
    std::string tempq = "PRAGMA table_info('";
    tempq.append(tableName->toStdString());
    tempq.append("')");
    const char* sql = tempq.c_str();

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *columnName = sqlite3_column_text(stmt, 1); // https://stackoverflow.com/a/20643403
        const unsigned char *columnDataType = sqlite3_column_text(stmt, 2); // https://stackoverflow.com/a/20643403
        result->push_back(QString::fromUtf8(reinterpret_cast<const char *>(columnName)));
        datatypes->push_back(types::dbTypeMap[QString::fromUtf8(reinterpret_cast<const char *>(columnDataType))]);
    }
    sqlite3_finalize(stmt);
}

// Fills tableRows from entries that exist for the requested platform in the db
void dbAccess::getEntriesForPlatform(QString *visibleTable, std::vector<tableRow> *tableContent)
{
    std::string sqls = "SELECT * FROM ";
    sqls.append(visibleTable->toStdString());
    sqls.append(" ORDER BY name ASC;");
    qDebug() << "getEntriesForPlatform " << sqls;
    const char* sql = sqls.c_str();

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    //TODO: make less hardcoded. Currently no way to know what column correspons to what member in tableRow
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tableRow newEntry;
        newEntry.id = sqlite3_column_int(stmt, 0);
        newEntry.name = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        newEntry.completed = sqlite3_column_int(stmt, 2);
        newEntry.platinum = sqlite3_column_int(stmt, 3);
        newEntry.pricePaid = sqlite3_column_double(stmt, 4);
        newEntry.notes = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5)));
        tableContent->push_back(newEntry);
    }
    sqlite3_finalize(stmt);
}

void dbAccess::addNewRowToDB(tableRow* newRow, QString* tableName)
{
    std::string completed = newRow->completed ? "1":"0";
    std::string platinum = newRow->platinum ? "1":"0";
    std::string priceStr = std::to_string(newRow->pricePaid);

    std::string sqls = "INSERT INTO " + tableName->toStdString() + "(name,completed,platinum,pricePaid,notes) VALUES('"
                      + newRow->name.toStdString() + "','" + completed + "','" + platinum + "','" + priceStr + "','" + newRow->notes.toStdString() + "');";
    qDebug() << sqls;

    const char* sql = sqls.c_str();
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErrMsg);
    qDebug() << rc << " " << zErrMsg;
}

void dbAccess::editRowInDb(tableRow* editedRow, QString* tableName)
{
    std::string completed = editedRow->completed ? "1":"0";
    std::string platinum = editedRow->platinum ? "1":"0";
    std::string priceStr = std::to_string(editedRow->pricePaid);

    std::string sqls = "UPDATE " + tableName->toStdString() + " " +
                       "SET name = '" + editedRow->name.toStdString() + "', " +
                           "completed = '" + completed + "', " +
                           "platinum = '" + platinum + "', " +
                           "pricePaid = '" + priceStr + "', " +
                           "notes = '" + editedRow->notes.toStdString() + "' " +
                       "WHERE id = " + std::to_string(editedRow->id) + ";";
    qDebug() << sqls;

    const char* sql = sqls.c_str();
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErrMsg);
    qDebug() << rc << " " << zErrMsg;
}

void dbAccess::deleteRowWithId(int* id, QString* tableName)
{
    std::string sqls = "DELETE FROM " + tableName->toStdString() + " " +
                       "WHERE id = " + std::to_string(*id) + ";";
    qDebug() << sqls;
    const char* sql = sqls.c_str();
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErrMsg);
    qDebug() << "deleteRowWithId() sql returned " << rc << " " << zErrMsg;
}
