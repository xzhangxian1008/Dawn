#pragma once

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "util/util.h"

namespace dawn {

class Task {
public:
    DISALLOW_COPY_AND_MOVE(Task);

    virtual ~Task() {};
    virtual void run();
};

// TODO Notify the parent thread to clear the closed fd
class HandleClientMsgTask : public Task {
public:
    DISALLOW_COPY_AND_MOVE(HandleClientMsgTask);

    HandleClientMsgTask(int fd) : fd_(fd) {}

    ~HandleClientMsgTask() override {
        if (fd_ != -1) {
            // TODO close fd in parent thread
        }
    }

    void run() override {
        while(true) {
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE-1, 0);
            if(ret < 0) {
                if((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    // Read later
                    break;
                }
                close( sockfd );
                // TODO close fd in parent thread and set fd to -1
                break;
            }
            else if (ret == 0) {
                // close( sockfd );
                // TODO close the fd in parent thread and set fd to -1
            }
            

        }
    }

private:
    int fd_;
};

} // dawn
