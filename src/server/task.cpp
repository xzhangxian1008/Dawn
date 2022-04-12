#include <memory>
#include <pair>
#include <unistd.h>

#include "server/task.h"
#include "sql/lex.h"
#include "sql/sql_parse.h"

namespace dawn {

void HandleClientMsgTask::run() override {
    while(true) {
        memset(buffer_, '\0', RECV_BUFFER_SIZE);
        int ret = recv(sockfd, buf, RECV_BUFFER_SIZE-1, 0);
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
        
        size_t_ semi_pos = search_semicolon();
        size_t push_num = semi_pos == -1 ? ret : semi_pos + 1;

        for (size_t i = 0; i < push_num; i++) {
            tmp_.push_back(buffer_[i]);
        }

        if (semi_pos != -1) {
            // We find a sql and handle it.
            handle_sql();
            tmp_.clear();

            // Push the remaining chars into tmp_
            for (size_t i = push_num; i < ret; i++) {
                tmp_.push_back(buffer_[i]);
            }
        }
    }
}

void HandleClientMsgTask::handle_sql() {
    tmp_.push(0);
    std::unique_ptr<string_t> sql = std::make_unique<string_t>(tmp_.data());
    Lex lex(std::move(sql));

    ResponseMsg resp_msg = sql_execute(lex);
    std::pair<const char*, int> msg = resp_msg.get_response_msg();

    if (write(fd_, msg.first, msg.second) < msg.second) {
        LOG("Write fail");
    }
}

size_t_ HandleClientMsgTask::search_semicolon() const {
    for (int i = 0; i < RECV_BUFFER_SIZE - 1; i++) {
        if (buffer_[i] == ';') {
            return i;
        }
    }

    return -1;
}

} // namespace dawn
