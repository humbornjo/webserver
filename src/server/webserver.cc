#include "webserver.h"

WebServer::WebServer(
    int port, int trig_mode, int timeout_ms, bool open_linger,
    int sql_port, const char *sql_user, const char *sql_pwd, 
    const char *dbname, int conn_pool_num, int thread_num, 
    bool open_log, int log_level, int log_que_size): 
    port_(port), timeout_ms_(timeout_ms), open_linger_(open_linger), is_close_(false),
    // TODOTODO
    epoller_(new Epoller())
    {
        src_dir_ = getcwd(nullptr, 256);
        assert(src_dir_); // incase getcwd() return NULL
        strncat(src_dir_, "/resources/", 16);
    }


void WebServer::InitEventMode(int trig_mode) {

}