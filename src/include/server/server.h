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
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <future>

#include "util/config.h"
#include "manager/db_manager.h"
#include "util/work_pool.h"

namespace dawn {

class Server {
public:
    DISALLOW_COPY_AND_MOVE(Server);

    Server(const string_t &addr, int port)
        : address_(addr), port_(port), wp_(SERVER_THD_NUMBER) {
        struct sockaddr_in address;

        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;

        inet_pton(AF_INET, address_.data(), &address.sin_addr);
        address.sin_port = htons(port);

        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        assert(listen_fd_ >= 0);

        int ret = bind(listen_fd_, (struct sockaddr*)&address, sizeof(address));
        assert(ret != -1);

        ret = listen(listen_fd_, 5);
        assert(ret != -1);

        ret = pipe(pipe_fd_);
        assert(ret != -1);

        wp_.start_up();

        fd_collect_thd_ = std::thread([this]{
            std::vector<decltype(conn_.begin())> delete_vec;
            std::unique_lock<std::mutex> ul(mut_, std::defer_lock);
            while (true) {
                delete_vec.clear();
                ul.lock();

                // Strange, why lock_guard won't release lock in while loop?
                // std::lock_guard<std::mutex> lg(mut_); 

                auto iter = conn_.begin();
                while (iter != conn_.end()) {
                    if (iter->second->is_finish()) {
                        delete iter->second;
                        delete_vec.push_back(iter);
                    }
                    iter++;
                }

                // Delete task from the map
                while (!delete_vec.empty()) {
                    iter = delete_vec.back();
                    delete_vec.pop_back();
                    conn_.erase(iter);
                }

                if (shutdown_) {
                    ul.unlock();    
                    break;
                }
                
                ul.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });
    }

    ~Server() {
        if (!shutdown_) {
            shutdown();
        }
    }
    
    void run();
    void shutdown();
private:
    int setnonblocking(int fd);
    void addfd(int fd, bool enable_et);

    /**
     * @return true continue the epoll_wait
     * @return false server may be shutdown and should no longer to enter into epoll_wait
     */
    bool process_epoll(epoll_event* events, int number);

    string_t address_;
    int port_;
    int listen_fd_;
    int epoll_fd_;

    // Key refers to the connection fd
    // Values refers to a task related with this connection
    std::map<int, Task*> conn_;

    // Protect the conn_
    std::mutex mut_;

    WorkPool wp_;
    std::thread fd_collect_thd_;

    bool shutdown_ = false;

    // Sometimes we need to wake server up from epoll_wait to do something
    int pipe_fd_[2];
};

} // namespace dawn
