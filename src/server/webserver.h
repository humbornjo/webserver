#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"

// tobe deleted header 
#include <string.h>
#include <memory>

class WebServer {
public:
    WebServer(
        //what is opt_linger
        int port, int trig_mode, int timeout_ms, bool open_linger,
        int sql_port, const char *sql_user, const char *sql_pwd, 
        const char *dbname, int conn_pool_num, int thread_num, 
        bool open_log, int log_level, int log_que_size);
    ~WebServer();
    void start();
private:
    bool InitSocket();
    void InitEventMode(int trig_mode);
    void AddClient(int fd, sockaddr_in addr);

    // member varables
    int port_;
    int timeout_ms_;
    bool open_linger_;
    bool is_close_;
    int listen_fd_;
    char *src_dir_;

    uint32_t listen_event_;
    uint32_t conn_event_;

    std::unique_ptr<Epoller> epoller_;

};



#endif // WEBSERVER_H