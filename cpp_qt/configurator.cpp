#include "configurator.h"

std::wstring getDbPath()
{
    WCHAR dbPath[500];
    GetPrivateProfileString(L"database", L"file", L"defaultdb.db", dbPath, 500, L".\\conf.ini");
    return std::wstring(dbPath);
}
