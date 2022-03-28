#pragma once

#include <stdio.h>
#include <string>
#include <memory>
#include <assert.h>

#include "util/config.h"
#include "sql/parse.h"

namespace dawn {

struct Token {
    int type_;
    union {
        boolean_t bool_,
        integer_t integer_,
        decimal_t decimal_,
        varchar_t varchar_
    } val_;
};

class Lex {
public:
    // Considering the convenience of implementation, Lex stores all content of file in the string.
    // TODO Implement this with buffer pool
    Lex(std::unique_ptr<FILE> yyin) : yyin_(std::move(yyin)) {
        string_t* sql = new string_t("");
        FILE* file = yyin.get();
        assert(file);

        const size_t buf_size = 128;
        char buf[buf_size];

        // Read content from file and convert them into string
        while (fgets(buf, buf_size, file)) {
            sql->append(buf);
        }

        sql_len_ = sql->length();
        sql_.reset(sql);
    }

    Lex(std::unique_ptr<string_t> sql) : sql_(std::move(sql)) {}

    bool next_token(Token* tk);
private:
    char next_char() {
        if (pos_ >= sql_len_) {
            return 0;
        }
        return sql_->at(pos_++);
    }

    std::unique_ptr<FILE> yyin_;
    std::unique_ptr<string_t> sql_;
    size_t sql_len_;
    size_t pos_ = 0;
};

} // namespace dawn
