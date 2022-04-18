#include <utility>

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

bool Server::process_epoll(epoll_event* events, int number) {
    for (int i = 0; i < number; i++) {
        int sock_fd = events[i].data.fd;
        LOG("HERE");

        if (sock_fd == pipe_fd_[0]) {
            LOG("HERE");
            std::lock_guard lg(mut_);
            LOG("HERE");
            shutdown_ = true;
            return false;
        }

        if (sock_fd == listen_fd_) {
            struct sockaddr_in client_address;
            socklen_t client_addr_length = sizeof(client_address);
            std::lock_guard lg(mut_);
            int conn_fd = accept(listen_fd_, (struct sockaddr*) &client_address, &client_addr_length);
            addfd(conn_fd, true);

            LOG("Receive client");
            conn_.insert(std::make_pair(conn_fd, new HandleClientMsgTask(conn_fd)));
        }
        else if (events[i].events & EPOLLIN) {
            LOG("HERE");
            std::lock_guard lg(mut_);
            LOG("HERE");
            auto iter = conn_.find(sock_fd);
            if (iter == conn_.end()) {
                FATAL("Encounter a fd that has never seen before.");
            }

            LOG("Dispatch EPOLLIN task");
            wp_.add_task(iter->second);
        }
        else {
            LOG("something else happened");
        }
    }

    return true;
}

void Server::run() {
    if (!shutdown_) {
        epoll_event events[MAX_EVENT_NUMBER];
        epoll_fd_ = epoll_create(5);
        assert(epoll_fd_ != -1);
        addfd(listen_fd_, true);
        addfd(pipe_fd_[0], true);

        LOG("Server enters into epoll_wait...");
        while(true) {
            int ret = epoll_wait(epoll_fd_, events, MAX_EVENT_NUMBER, -1);
            LOG("epoll is waked up");
            if (ret < 0) {
                LOG("Epoll failure");
                break;
            }
        
            if (!process_epoll(events, ret)) {
                break;
            }
        }

        close(epoll_fd_);
    }
}

void Server::shutdown() {
    if (shutdown_) return;
LOG("shutdown");
    {
        std::lock_guard lg(mut_);
        wp_.wait_until_all_finished();
        LOG("shutdown");
        auto iter = conn_.begin();
        while (iter != conn_.end()) {
            // Wait for the end of the task
            while (true) {
                if (iter->second->is_finish()) {
                    break;
                }
                LOG("shutdown");
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            delete iter->second;
        }
LOG("shutdown");
        // Delete listen_fd_ from epoll
        struct epoll_event event;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, listen_fd_, &event) != 0) {
            FATAL("epoll_ctl error");
        }
LOG("shutdown");
        // Deny any client that tries to connect to the server
        close(listen_fd_);
LOG("shutdown");
        char c = 'c';
        if (write(pipe_fd_[1], &c, 1) != 0) {
            FATAL("Pipe Error");
        }
    }

    LOG("shutdown");

    std::unique_lock<std::mutex> ul(mut_, std::defer_lock);
    while (true) {
        ul.lock();
        if (shutdown_) {
            ul.unlock();
            break;
        }
        ul.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
LOG("shutdown");
    if (fd_collect_thd_.joinable()) {
        fd_collect_thd_.join();
    } else {
        FATAL("fd_collect_thd_ is unjoinable");
    }
}


} // namespace dawn
