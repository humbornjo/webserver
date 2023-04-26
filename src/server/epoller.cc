
#include "epoller.h"

Epoller::Epoller(int max_events):
    epollfd_(epoll_create(1024)), events_(max_events){
        assert(events_.size() > 0 && epollfd_ > 0);
}

Epoller::~Epoller() {
    close(epollfd_);
}