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

    // add file descriptor to epoller
    bool AddFd(int fd, uint32_t events);

    // modify file descriptor in epoller
    bool ModFd(int fd, uint32_t events);

    // delete file descriptor in epoller
    bool DelFd(int fd);

    // return the fd of the i th fd in events_, const enssure robustness
    int GetEventFd(size_t i) const;

    // return the event type of the i th fd in events_, const enssure robustness
    uint32_t GetEvent(size_t i) const;

    int Wait();
};

#endif //EPOLLER_H