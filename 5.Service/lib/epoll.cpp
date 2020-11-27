#include <cstring>
#include "epoll.h"
#include "neterr.h"

namespace {

void handle_error(int err) {
    if (err < 0) {
        throw net::EPollError(std::strerror(errno));
    }
}

constexpr int MAX_EVENTS = 1000;

void try_set_epoll(int epfd, int epollopt, int fd, ::epoll_event& event) {
    int error = epoll_ctl(epfd, epollopt, fd, &event);
    handle_error(error);
}

}  // namespace


namespace net {

EPoll::EPoll()
    : events_queue_size_(MAX_EVENTS)
    , epoll_fd_(::epoll_create(1)) {
    handle_error(epoll_fd_.fd());
}

void EPoll::set_max_events(size_t max_events) {
    events_queue_size_ = max_events;
}

void EPoll::add(const tcp::Descriptor& d, OPTION opt) const {
    ::epoll_event event{};
    event.events  = static_cast<uint32_t>(opt) | EPOLLRDHUP;
    event.data.fd = d.fd();
    try_set_epoll(epoll_fd_.fd(), EPOLL_CTL_ADD, d.fd(), event);
}

void EPoll::add(tcp::Connection* cn, OPTION opt) const {
    ::epoll_event event{};
    event.events  = static_cast<uint32_t>(opt) | EPOLLRDHUP;
    event.data.ptr = cn;
    try_set_epoll(epoll_fd_.fd(), EPOLL_CTL_ADD, cn->fd().fd(), event);
}

void EPoll::mod(const tcp::Descriptor& d, OPTION opt) const {
    ::epoll_event event{};
    event.events  = static_cast<uint32_t>(opt) | EPOLLRDHUP;
    event.data.fd = d.fd();
    try_set_epoll(epoll_fd_.fd(), EPOLL_CTL_MOD, d.fd(), event);
}

void EPoll::mod(tcp::Connection* cn, OPTION opt) const {
    ::epoll_event event{};
    event.events  = static_cast<uint32_t>(opt) | EPOLLRDHUP;
    event.data.ptr = cn;
    try_set_epoll(epoll_fd_.fd(), EPOLL_CTL_MOD, cn->fd().fd(), event);
}

void EPoll::del(const tcp::Descriptor& d) const {
    int error = epoll_ctl(epoll_fd_.fd(), EPOLL_CTL_DEL, d.fd(), nullptr);
    handle_error(error);
}

std::vector<::epoll_event> EPoll::wait() const {
    std::vector<::epoll_event> event_queue(events_queue_size_);
    int events_count = ::epoll_wait(epoll_fd_.fd(),
                 event_queue.data(), event_queue.size(), -1);
    handle_error(events_count);
    event_queue.resize(events_count);
    return event_queue;
}

}  // namespace net