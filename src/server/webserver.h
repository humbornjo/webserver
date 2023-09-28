#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h> // fcntl()
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> // close()
#include <unordered_map>

#include "../http/httpconn.h"
#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../timer/heaptimer.h"
#include "epoller.h"

class WebServer {
public:
  WebServer(
      // what is opt_linger
      int port, int trig_mode, int timeout_ms, bool open_linger, int sql_port,
      const char *sql_user, const char *sql_pwd, const char *dbname,
      int conn_pool_num, int thread_num, bool open_log, int log_level,
      int log_que_size);
  ~WebServer();
  void start();

private:
  bool InitSocket_();
  void InitEventMode_(int trig_mode);
  void AddClient_(int fd, sockaddr_in addr);

  void DealListen_();
  void DealWrite_(HttpConn *client);
  void DealRead_(HttpConn *client);

  void SendError_(int fd, const char *info);
  void ExtentTime_(HttpConn *client);
  void CloseConn_(HttpConn *client);

  void OnRead_(HttpConn *client);
  void OnWrite_(HttpConn *client);
  void OnProcess(HttpConn *client);

  static const int MAX_FD = 65536;

  static int SetFdNonblock(int fd);

  // member varables
  int port_;
  int timeout_ms_;
  bool optlinger_;
  bool is_close_;
  int listen_fd_;
  char *src_dir_;

  uint32_t listen_event_;
  uint32_t conn_event_;

  std::unique_ptr<HeapTimer> timer_;
  std::unique_ptr<ThreadPool> threadpool_;
  std::unique_ptr<Epoller> epoller_;
  std::unordered_map<int, HttpConn> users_;
};

#endif // WEBSERVER_H
