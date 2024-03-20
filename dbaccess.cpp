#include "dbaccess.h"
#include <iostream>

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
