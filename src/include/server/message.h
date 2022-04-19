#pragma once

#include <utility>

#include "utility"

namespace dawn {

enum MsgType {
    Unparsed,
    SqlInsert,
    SqlSelect,
    SqlCreateTb,
    SqlDropTb,
    SqlDelete
};

class ResponseMsg {
public:
    ResponseMsg(MsgType type) : type_(type), success_(false) { set_prefix(); }

    inline void set_type(MsgType type) {
        type_ = type;
        set_prefix();
    }

    inline MsgType get_type() const {
        return type_;
    }

    inline bool is_success() const {
        return success_;
    }

    inline void append_msg(const string_t& msg) {
        msg_.append(msg);
    }

    inline void set_status(bool success) {
        success_ = success;
        if (success) {
            prefix_.append(" OK.");
        } else {
            prefix_.append(" FAIL.");
        }
    }

    inline string_t get_response_msg() const {
        return prefix_ + msg_;
    }
private:
    void set_prefix() {
        switch (type_) {
        case MsgType::SqlDelete:
            prefix_ = string_t("Delete");
            break;
        case MsgType::SqlInsert:
            prefix_ = string_t("Insert");
            break;
        case MsgType::SqlCreateTb:
            prefix_ = string_t("Create Table");
            break;
        case MsgType::SqlDropTb:
            prefix_ = string_t("Drop Table");
            break;
        case MsgType::Unparsed:
            prefix_ = string_t("Sql is invalid.");
            break;
        default:
            break;
        }
    }

    MsgType type_;
    string_t prefix_;
    string_t msg_;
    bool success_;
};

} // namespace dawn
