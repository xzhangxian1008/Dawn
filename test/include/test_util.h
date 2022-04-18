#pragma once

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory>
#include <thread>

#include "util/util.h"

class TestParam {
public:
    TestParam(std::string tcase) : tcase_(tcase) {}
    std::string get_tcase() const { return tcase_; }
private:
    std::string tcase_; // test case file
};

#define SA struct sockaddr

class Client {
public:
    Client(const std::string& address, int port)
        : address_(address), port_(port) {
        struct sockaddr_in servaddr;
        const char* addr = address_.data();

        sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd_ <= 0) {
            dawn::FATAL("sock_fd_ <= 0");
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, addr, &servaddr.sin_addr) != 1) {
            dawn::FATAL("inet_pton invalid");
        }

        if (connect(sock_fd_, (SA *) &servaddr, sizeof(servaddr)) != 0) {
            dawn::FATAL("connect fail");
        }
    }

    ~Client() {
        dawn::LOG("Client closes sock_fd_");
        close(sock_fd_);
    }

    /**
     * @return true is returned when receiving response from db.
     * @param last Set to true means this msg is the end of a sql and
     * procedure will be blocked for waiting messages returned from db.
     */
    inline bool send(const std::string& msg, bool last = false) {

        auto write_num = write(sock_fd_, msg.data(), msg.length());
        // dawn::PRINT(write_num);
        if (write_num < static_cast<ssize_t>(msg.length())) {
            dawn::FATAL("write < length");
        }

        if (last) {
            int buf_size = 1024;
            char buf[buf_size]; // Suppose we can receive all data with only one buffer
            memset(buf, 0, buf_size);

            int flag = 0;
            flag |= MSG_DONTWAIT;

            // DB should return results in 500ms
            int count = 5;
            while (count >= 0) {
                int ret = recv(sock_fd_, buf, buf_size, flag);
                if (ret > 0) {
                    resp_ = std::string(buf);
                    return true;
                } else if (ret < 0) {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                        // read later
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        count--;
                        dawn::LOG("count--");
                        continue;
                    }
                    switch (errno)
                    {
                    case EBADF:
                        dawn::LOG("EBADF");
                        break;
                    case ECONNREFUSED:
                        dawn::LOG("ECONNREFUSED");
                        break;
                    case EFAULT:
                        dawn::LOG("EFAULT");
                        break;
                    case EINTR:
                        dawn::LOG("EINTR");
                        break;
                    case EINVAL:
                        dawn::LOG("EINVAL");
                        break;
                    case ENOMEM:
                        dawn::LOG("ENOMEM");
                        break;
                    case ENOTCONN:
                        dawn::LOG("ENOTCONN");
                        break;
                    case ENOTSOCK:
                        dawn::LOG("ENOTSOCK");
                        break;
                    
                    default:
                    dawn::LOG("default");
                        break;
                    }
                    dawn::FATAL("client ret < 0");
                } else if (ret == 0) {
                    dawn::FATAL("client ret == 0");
                }
            }
        }
        return false;
    }

    std::string get_db_response() const {
        return resp_;
    }
private:
    std::string address_;
    int port_;
    int sock_fd_;
    std::string resp_;
};
