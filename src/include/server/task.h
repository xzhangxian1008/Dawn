#pragma once

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <vector>
#include <string>
#include <atomic>

#include "util/util.h"

namespace dawn {

class Task {
public:
    DISALLOW_COPY_AND_MOVE(Task);

    Task() = default;
    virtual ~Task() {};
    virtual void run() = 0;
    virtual bool is_finish() const = 0;
};

class HandleClientMsgTask : public Task {
public:
    DISALLOW_COPY_AND_MOVE(HandleClientMsgTask);

    HandleClientMsgTask(int fd) : fd_(fd), finish_(false) {
        tmp_.reserve(RECV_BUFFER_SIZE / 4);
    }

    ~HandleClientMsgTask() override {
        if (fd_ != -1) {
            close(fd_);
        }
    }

    bool is_finish() const override {
        return finish_.load();
    }

    void run() override;

private:
    // TODO Semicolon may appear in the string, we should handle this kind of situation
    size_t_ search_semicolon() const;

    void handle_sql();
    void response();

    /** Refer to the connection with a client */
    int fd_;

    /** Store string that read at the last time */
    std::vector<char> tmp_;

    /** Receive data and put them into this buffer */
    char buffer_[RECV_BUFFER_SIZE];

    std::atomic_bool finish_;
};

} // dawn
