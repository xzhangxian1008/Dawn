#pragma once

#include <stdio.h>
#include <string>
#include <memory>
#include <assert.h>
#include <vector>
#include <map>
#include <cstring>
#include <set>

#include "util/config.h"
#include "sql/parse.h"

namespace dawn {

struct Token {
    int type_;
    union {
        integer_t integer_;
        decimal_t decimal_;
        varchar_t varchar_;
    } val_;

    // size includes the end 0;
    void build_str(const char* source, size_t size) {
        val_.varchar_ = new char[size];
        memcpy(val_.varchar_, source, size);
    }
};

class Lex {
public:
    // Considering the convenience of implementation, Lex stores all content of file in the string.
    // TODO Implement this with buffer pool
    Lex(FILE* input_file) : input_file_(input_file) {
        string_t* sql = new string_t("");

        const size_t buf_size = 128;
        char buf[buf_size];

        // Read content from file and convert them into string
        while (fgets(buf, buf_size, input_file_)) {
            sql->append(buf);
        }

        sql_len_ = sql->length();
        sql_.reset(sql);
    }

    Lex(std::unique_ptr<string_t> sql) : sql_(std::move(sql)) {}

    ~Lex() {
        fclose(input_file_);
    }

    bool next_token(Token* tk);
private:
    void back() { pos_--; }

    // Get char referred by the current pos_
    char get_char() const { return sql_->at(pos_); }

    char next_char() {
        if (pos_ + 1 >= sql_len_) {
            return 0;
        }
        return sql_->at(++pos_);
    }

    char peek_next() {
        if (pos_ + 1 >= sql_len_) {
            return 0;
        }
        return sql_->at(pos_+1);
    }

    bool is_key_word(Token* tk) const;

    // Some key words may have space such as "PRIMARY KEY",
    // we need to process these special cases.
    bool is_special_key_word(Token* tk);

    // Check if we should call is_special_key_word() function.
    bool check_special_key_word_prefix(Token* tk);

    bool read_id(Token* tk);
    bool read_number(Token* tk);
    bool read_string(Token* tk);
    bool read_decimal(Token* tk);

    FILE* input_file_;
    std::unique_ptr<string_t> sql_;
    size_t_ sql_len_;
    size_t_ pos_ = -1;

    // Some chars need to be stored in somewhere temporarily
    std::vector<char> buf_;
};

} // namespace dawn
