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

        fd_collect_thd_ = std::thread([this]{
            std::vector<decltype(conn_.begin())> delete_vec;
            while (true) {
                delete_vec.clear();
                std::lock_guard<std::mutex> lg(mut_);

                auto iter = conn_.begin();
                while (iter != conn_.end()) {
                    if (iter->second->is_finish()) {
                        delete iter->second;
                        delete_vec.push_back(iter);
                    }
                }

                // Delete task from the map
                while (!delete_vec.empty()) {
                    iter = delete_vec.back();
                    delete_vec.pop_back();
                    conn_.erase(iter);
                }

                if (shutdown_)
                    break;

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });
    }

    ~Server() {
        {
            std::lock_guard lg(mut_);
            wp_.wait_until_all_finished();
            
            auto iter = conn_.begin();
            while (iter != conn_.end()) {
                // Wait for the end of the task
                while (true) {
                    if (iter->second->is_finish()) {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                delete iter->second;
            }

            shutdown_ = true;
        }

        fd_collect_thd_.join();
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

    // Protect the conn_
    std::mutex mut_;

    WorkPool wp_;
    std::thread fd_collect_thd_;

    bool shutdown_ = false;
    
};

} // namespace dawn
