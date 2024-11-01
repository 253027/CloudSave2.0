#include "mysql.h"
#include <sstream>
#include <string.h>

mg::Mysql::Mysql()
{
    ;
}

mg::Mysql::~Mysql()
{
    ;
}

std::string mg::Mysql::parseInsert(const std::string &name, mysql::DataField *column, char *data)
{
    using DATATYPE = mysql::DATATYPE;
    using CALCTYPE = mysql::CALCTYPE;
    int offset = 0;
    std::ostringstream sql;
    sql << "insert into `" << name << "` (";
    std::string field, value;
    for (mysql::DataField *p = column; p->name; p++)
    {
        if (!::strlen(p->name))
            continue;
        field += p->name, field += ", ";
        int len = 0;
        switch (p->type)
        {
        case DATATYPE::DB_STRING:
        {
            len = ::strlen(data + offset);
            if (p->size != CALCTYPE::DB_AUTO)
                len = std::min(len, p->size);
            std::string buf(len * 2 + 3, '\'');
            len = mysql_escape_string(buf.data() + 1, data + offset, len);
            buf.resize(len + 2);
            buf.back() = '\'';
            value += buf;
            break;
        }
        case DATATYPE::DB_INT32:
        {
            value += std::to_string(*((int *)(data + offset)));
            break;
        }
        }
        value += ", ";
        offset += p->size ? p->size : len;
    }
    field = field.substr(0, field.size() - 2);
    value = value.substr(0, value.size() - 2);
    sql << field << ") values (" << value << ")";
    return sql.str();
}
