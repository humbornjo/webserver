#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

class Epoller {
private:
    int epollfd_;
    std::vector<struct epoll_event> events_;
public:
    explicit Epoller(int max_events = 1024);
    ~Epoller();  
};

#endif //EPOLLER_H