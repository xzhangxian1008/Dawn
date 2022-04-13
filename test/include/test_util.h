#pragma once

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
    Client(const std::string& address, int port) : address_(address), port_(port) {
        int sockfd;
        struct sockaddr_in servaddr;
        const char* addr = address_.data();

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd <= 0) {
            dawn::FATAL("sockfd <= 0");
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, addr, &servaddr.sin_addr) != 1) {
            dawn::FATAL("inet_pton invalid");
        }

        if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) != 0) {
            dawn::FATAL("connect fail");
        }
    }

    void send(const std::string& msg) const {

    }

    std::string get_db_response() const {

    }
private:
    std::string address_;
    int port_;
};
