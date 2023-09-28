#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include "../log/log.h"
#include <mutex>
#include <mysql/mysql.h>
#include <queue>
#include <semaphore.h>
#include <string>
#include <thread>

class SqlConnPool {
public:
  static SqlConnPool *Instance();

  MYSQL *GetConn();
  void FreeConn(MYSQL *conn);
  int GetFreeConnCount();

  void Init(const char *host, int port, const char *user, const char *pwd,
            const char *dbname, int conn_size = 10);
  void ClosePool();

private:
  SqlConnPool();
  ~SqlConnPool();

  int MAX_CONN_;
  int use_count_;
  int free_count_;

  std::queue<MYSQL *> conn_que_;
  std::mutex mtx_;
  sem_t sem_id_;
};

#endif // SQLCONNPOOL_H
