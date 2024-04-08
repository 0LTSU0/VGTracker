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
                                 {"BOOLEAN", types::columnType::boolean},
                                 {"REAL", types::columnType::decimal}};

}

// datastructure that corresponds to format of table
class tableRow
{
public:
    int id;
    QString name;
    bool completed;
    bool platinum;
    float pricePaid;
    QString notes;
    bool _isMarkedForDelete = false;
};

// define max int to mean row id thats not yet saved in db
inline int notYetInDb = 2147483647;

#endif // ENUMS_H
