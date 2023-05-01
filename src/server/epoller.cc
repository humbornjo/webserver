#include "epoller.h"

Epoller::Epoller(int max_events):
    epollfd_(epoll_create(1024)), events_(max_events){
        assert(events_.size() > 0 && epollfd_ > 0);
}

Epoller::~Epoller() {
    close(epollfd_);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event epev = {0};
    epev.data.fd = fd;
    epev.events = events;
    return 0 == epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &epev);
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event epev = {0};
    epev.data.fd = fd;
    epev.events = events;
    return 0 == epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &epev);
}

bool Epoller::DelFd(int fd) {
    if (fd < 0) return false;
    epoll_event epev = {0};  // object-oriented standard, cpp style
    return 0 == epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &epev);
}

int Epoller::GetEventFd(size_t i) const {
    assert(i>=0 && i<events_.size());
    return events_[i].data.fd;
}

uint32_t Epoller::GetEvent(size_t i) const {
    assert(i>=0 && i<events_.size());
    return events_[i].events;
}

int Epoller::Wait() {
    epoll_wait(epollfd_, &events_[0], static_cast<int>(events_.size()), -1);
}