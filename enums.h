#ifndef ENUMS_H
#define ENUMS_H

#include <QString>
#include <map>

namespace types
{

enum columnType
{
    integer = 0,
    text = 1,
    boolean = 2,
    decimal = 3
};

inline std::map<QString, int> dbTypeMap{{"INTEGER", types::columnType::integer},
                                 {"TEXT", types::columnType::text, },
                                 {"BOOL", types::columnType::boolean},
                                 {"FLOAT", types::columnType::decimal}};

}

#endif // ENUMS_H
