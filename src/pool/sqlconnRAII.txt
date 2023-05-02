#include "sqlconnRAII.h"

SqlConnRAII::SqlConnRAII(MYSQL **sql, SqlConnPool *connpool) {
    assert(connpool);
    *sql = connpool->GetConn();
    sql_ = *sql;
    connpool_ = connpool;
}

SqlConnRAII::~SqlConnRAII() {
    if (sql_) {
        connpool_->FreeConn(sql_);
    }
}