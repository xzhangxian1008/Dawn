#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <memory>
#include <sys/epoll.h>
#include <fcntl.h>
#include <map>

#include "util/config.h"
#include "manager/db_manager.h"
#include "util/work_pool.h"

namespace dawn {

// TODO Start a thread to collect closed fds
class Server {
public:
    DISALLOW_COPY_AND_MOVE(Server);

    Server(const string_t &addr, int port)
        : address_(addr), port_(port), wp_(SERVER_THD_NUMBER) {
        int ret = 0;
        struct sockaddr_in address;

        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;

        inet_pton(AF_INET, address_.data(), &address.sin_addr);
        address.sin_port = htons(port);

        listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
        assert(listen_fd_ >= 0);

        ret = bind(listen_fd_, (struct sockaddr*)&address, sizeof(address));
        assert(ret != -1);

        ret = listen(listen_fd_, 5);
        assert(ret != -1);

        wp_.start_up();
    }

    ~Server() {
        wp_.wait_until_all_finished();
        
        auto iter = conn_.begin();
        while (iter != conn_.end()) {
            delete iter->second;
        }
    }
    
    void run();
private:
    int setnonblocking(int fd);
    void addfd(int fd, bool enable_et);
    void process_epoll(epoll_event* events, int number);

    string_t address_;
    int port_;
    int listen_fd_;
    int epoll_fd_;

    // Key refers to the connection fd
    // Values refers to a task related with this connection
    std::map<int, Task*> conn_;

    WorkPool wp_;
};

} // namespace dawn
