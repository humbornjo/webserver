#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H
#include "sqlconnpool.h"

class SqlConnRAII {
public:
    SqlConnRAII(MYSQL **sql, SqlConnPool *connpool);
    ~SqlConnRAII();
private:
    MYSQL *sql_;
    SqlConnPool* connpool_;
};

#endif // SQLCONNRAII_H