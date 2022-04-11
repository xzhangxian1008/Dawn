#include "server/server.h"
#include "util/util.h"

namespace dawn {

int Server::setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void Server::addfd(int fd, bool enable_et) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et) {
        event.events |= EPOLLET;
    }
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void Server::process_epoll(epoll_event* events, int number) {
    for (int i = 0; i < number; i++) {
        int sock_fd = events[i].data.fd;
        if (sock_fd == listen_fd_) {
            struct sockaddr_in client_address;
            socklen_t client_addr_length = sizeof(client_address);
            int conn_fd = accept(listen_fd_, (struct sockaddr*) &client_address, &client_addr_length);
            addfd(conn_fd, true);

            conn_.insert(std::make_pair<int, Task*>(conn_fd, new HandleClientMsgTask(conn_fd)));
        }
        else if (events[i].events & EPOLLIN) {
            auto iter = conn_.find(sock_fd);
            if (iter == conn_.end()) {
                FATAL("Encounter a fd that has never seen before.")
            }

            wp_.add_task(iter->second);
        }
        else {
            LOG("something else happened");
        }
    }
}

void Server::run() {
    epoll_event events[MAX_EVENT_NUMBER];
    epoll_fd_ = epoll_create(5);
    assert(epoll_fd_ != -1);
    addfd(listen_fd_, true);

    while(true) {
        int ret = epoll_wait(epoll_fd_, events, MAX_EVENT_NUMBER, -1);
        if (ret < 0) {
            LOG("Epoll failure");
            break;
        }
    
        process_epoll(epoll_event, ret);
    }

    close(listen_fd_);
}


} // namespace dawn
