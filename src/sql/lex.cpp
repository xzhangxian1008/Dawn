#include "sql/lex.h"
#include <stdlib.h>
#include "util/util.h"

namespace dawn {

static std::map<string_t, int> key_words{
    {"create", TOKEN_CREATE}, {"CREATE", TOKEN_CREATE},
    {"table", TOKEN_TABLE}, {"TABLE", TOKEN_TABLE},
    {"primary key", TOKEN_PRIMARY_KEY}, {"PRIMARY KEY", TOKEN_PRIMARY_KEY},
    {"char", TOKEN_CHAR}, {"CHAR", TOKEN_CHAR},
    {"int", TOKEN_INT}, {"INT", TOKEN_INT},
    {"decimal", TOKEN_DECIMAL}, {"DECIMAL", TOKEN_DECIMAL},
    {"boolean", TOKEN_BOOLEAN}, {"BOOLEAN", TOKEN_BOOLEAN},
    {"drop", TOKEN_DROP}, {"DROP", TOKEN_DROP},
    {"insert", TOKEN_INSERT}, {"INSERT", TOKEN_INSERT},
    {"into", TOKEN_INTO}, {"INTO", TOKEN_INTO},
    {"values", TOKEN_VALUES}, {"VALUES", TOKEN_VALUES},
    {"delete", TOKEN_DELETE}, {"DELETE", TOKEN_DELETE},
    {"from", TOKEN_FROM}, {"FROM", TOKEN_FROM},
    {"where", TOKEN_WHERE}, {"WHERE", TOKEN_WHERE},
    {"select", TOKEN_SELECT}, {"SELECT", TOKEN_SELECT},
    {"true", TOKEN_TRUE}, {"TRUE", TOKEN_TRUE},
    {"false", TOKEN_FALSE}, {"FALSE", TOKEN_FALSE},
    {"natural", TOKEN_NATURAL}, {"NATURAL", TOKEN_NATURAL},
    {"join", TOKEN_JOIN}, {"JOIN", TOKEN_JOIN},
    {"or", TOKEN_OR}, {"OR", TOKEN_OR},
    {"not", TOKEN_NOT}, {"NOT", TOKEN_NOT},
    {"and", TOKEN_AND}, {"AND", TOKEN_AND}
};

static std::set<string_t> key_words_prefix{"primary", "PRIMARY"};

bool Lex::is_key_word(Token* tk) const {
    string_t str(buf_.data());
    auto iter = key_words.find(str);
    if (iter == key_words.end()) {
        return false;
    }
    tk->type_ = iter->second;
    return true;
}

bool Lex::check_special_key_word_prefix(Token* tk) {
    buf_.push_back(0);
    string_t str(buf_.data());

    auto iter = key_words_prefix.find(str);
    if (iter == key_words_prefix.end()) {
        return false; // Can't be matched with any prefix of special key words
    }
    buf_.pop_back();
    return true;
}

bool Lex::is_special_key_word(Token* tk) {
    char c = next_char();
    while (c == ' ') c = next_char();
    buf_.push_back(' ');
    buf_.push_back(c);

    while (true) {
        c = next_char();
        switch (c) {
        case 'a' ... 'z': case 'A' ... 'Z':
        case '0' ... '9': case '_':
            buf_.push_back(c);
            break;
        default:
            back();
            goto out;
        }
    }

out:
    buf_.push_back(0);
    return is_key_word(tk);
}

bool Lex::read_string(Token* tk) {
    buf_.clear();
    char c = next_char();

    while (c != 0) {
        if (c == '"' || c == '\'') {
            buf_.push_back(0);
            tk->type_ = TOKEN_STRING;
            tk->build_str(buf_.data(), buf_.size());
            return true;
        } else if (c == '\\') {
            c = next_char();
            buf_.push_back(c);
        } else {
            buf_.push_back(c);
        }
        c = next_char();
    }
    return false;
}

bool Lex::read_id(Token* tk) {
    buf_.clear();
    buf_.push_back(get_char());
    
    while (true) {
        char c = next_char();
        switch (c) {
        case 'a' ... 'z': case 'A' ... 'Z':
        case '0' ... '9': case '_':
            buf_.push_back(c);
            continue;
        case ' ':
            if (!check_special_key_word_prefix(tk)) {
                back();
                goto out;
            }
            if (is_special_key_word(tk)) return true;
            return false;
        default:
            back();
            goto out;
        }
    }

out:
    buf_.push_back(0); // add 0 for string as the end for futher operation
    if (is_key_word(tk)) return true;

    tk->type_ = TOKEN_ID;
    tk->build_str(buf_.data(), buf_.size());
    return true;
}

bool Lex::read_number(Token* tk) {
    buf_.clear();
    buf_.push_back(get_char());

    while (true) {
        char c = next_char();
        switch (c) {
        case '0' ... '9':
            buf_.push_back(c);
            continue;
        case '.': {
            char next_c = peek_next();
            if (next_c >= '0' && next_c <= '9') {
                buf_.push_back('.');
                return read_decimal(tk);
            }
            return false;
        }
        default:
            back();
            goto out;
        }
    }

out:
    buf_.push_back(0); // add 0 for number string as the end for futher conversion
    tk->val_.integer_ = atof(buf_.data());

    if (tk->type_ != TOKEN_DECIMAL) tk->type_ = TOKEN_INT_NUM;
    return true;
}

bool Lex::read_decimal(Token* tk) {
    while (true) {
        char c = next_char();
        if (c >= '0' && c <= '9') buf_.push_back(c);
        else {
            back();
            break;
        }
    }
    buf_.push_back(0);
    tk->val_.decimal_ = atof(buf_.data());
    tk->type_ = TOKEN_DECIMAL_NUM;
    return true;
}

bool Lex::next_token(Token* tk) {
    char c = next_char();
    while (c == ' ' || c== '\n') {
        c = next_char();
    }

    if (c == 0) return false;

    switch (c) {
    case '(': { tk->type_ = TOKEN_LEFT_PARENTHESES; return true;}
    case ')': { tk->type_ = TOKEN_RIGHT_PARENTHESES; return true;}
    case ';': { tk->type_ = TOKEN_SEMICOLON; return true;}
    case ',': {tk->type_ = TOKEN_COMMA; return true;}
    case '<': {
        char next_c = next_char();
        if (next_c == '=') {
            tk->type_ = TOKEN_LE_EQ;
        } else {
            tk->type_ = TOKEN_LESS_SIGN;
            back();
        }
        return true;
    }
    case '>': {
        char next_c = next_char();
        if (next_c == '=') {
            tk->type_ = TOKEN_GT_EQ;
        } else {
            tk->type_ = TOKEN_GREATER_SIGN;
            back();
        }
        return true;
    }
    case '*': {tk->type_ = TOKEN_STAR; return true;}
    case '=': {tk->type_ = TOKEN_EQUAL; return true;}
    case '"': case '\'':
        return read_string(tk);
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '_':
        return read_id(tk);
    case '0' ... '9':
        return read_number(tk);
    default:
        break;
    }

    return false;
}

} // namespace dawn
