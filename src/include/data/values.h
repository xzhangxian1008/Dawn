#pragma once

#include <string>
#include <string.h>

#include "data/types.h"
#include "util/config.h"
#include "util/util.h"

namespace dawn {
enum class TypeId;
class Type;

extern Type *singleton[4];

class Value {
public:
    Value();
    Value(boolean_t val);
    Value(integer_t val);
    Value(decimal_t val);
    Value(char *val, int size);
    Value(const string_t &val);
    Value(char *value, TypeId type_id, size_t_ str_size = -1);
    ~Value();

    // deep copy
    Value(const Value &value) {
        this->type_id_ = value.type_id_;
        this->str_size_ = value.str_size_;
        
        if (value.type_id_ != TypeId::CHAR) {
            this->value_ = value.value_;
        } else {
            this->value_.char_ = new char[str_size_+1];
            memset(this->value_.char_, 0, str_size_ + 1);
            memcpy(this->value_.char_, value.value_.char_, str_size_ + 1);
        }
    }

    // deep copy
    Value& operator=(const Value &value) {
        if (this->type_id_ == TypeId::CHAR) {
            delete[] value_.char_;
        }

        this->type_id_ = value.type_id_;
        this->str_size_ = value.str_size_;

        if (value.type_id_ != TypeId::CHAR) {
            this->value_ = value.value_;
        } else {
            this->value_.char_ = new char[str_size_+1];
            memset(this->value_.char_, 0, str_size_ + 1);
            memcpy(this->value_.char_, value.value_.char_, str_size_ + 1);
        }

        return *this;
    }

    bool operator==(const Value &value) {
        if (this->type_id_ != value.type_id_)
            return false;
            
        bool ok = true;
        switch (value.type_id_) {
            case TypeId::INTEGER:
                if (this->value_.integer != value.value_.integer)
                    ok = false;
                break;
            case TypeId::BOOLEAN:
                if (this->value_.boolean != value.value_.boolean)
                    ok = false;
                break;
            case TypeId::DECIMAL:
                if (this->value_.decimal != value.value_.decimal)
                    ok = false;
                break;
            case TypeId::CHAR:
                if (string_t(this->value_.char_) != string_t(value.value_.char_)) 
                    ok = false;
                break;
            default:
                ok = false;
                break;
        }
        return ok;
    }

    void swap(Value &val) {
        std::swap(val.type_id_, type_id_);
        std::swap(val.value_, value_);
        std::swap(val.str_size_, str_size_);
    }

    // similar to shallow copy
    void load(const Value &val) {
        if (type_id_ == TypeId::CHAR)
            delete[] value_.char_;
        type_id_ = val.type_id_;
        value_ = val.value_;
        str_size_ = val.str_size_;
    }

    TypeId get_type_id() const { return type_id_; }

    template<typename T>
    T get_value() const {
        return *reinterpret_cast<const T*>(&value_);
    }

    int get_char_size() const { return str_size_; }

    // string should end with '\0'
    inline void serialize_to(char *storage) {
        if (type_id_ == TypeId::CHAR) {
            singleton[static_cast<int>(type_id_)]->serialize_to(storage, value_.char_);
            return;
        }
        singleton[static_cast<int>(type_id_)]->serialize_to(storage, (char*)(&value_));
    }

    inline void deserialize_from(char *src) {
        if (type_id_ == TypeId::CHAR) {
            singleton[static_cast<int>(type_id_)]->deserialize_from(value_.char_, src);
            return;
        }
        singleton[static_cast<int>(type_id_)]->deserialize_from((char*)(&value_), src);
    }

    hash_t get_hash_value() const {
        switch (type_id_) {
            case TypeId::BOOLEAN:
                boolean_t *val = const_cast<boolean_t*>(&(value_.boolean));
                return do_hash(static_cast<void*>(val), BOOLEAN_T_SIZE);
            case TypeId::INTEGER:
                integer_t *val = const_cast<integer_t*>(&(value_.integer));
                return do_hash(static_cast<void*>(val), INTEGER_T_SIZE);
            case TypeId::DECIMAL:
                decimal_t *val = const_cast<decimal_t*>(&(value_.decimal));
                return do_hash(static_cast<void*>(val), DECIMAL_T_SIZE);
            case TypeId::CHAR:
                char *val = const_cast<char*>(value_.char_);
                return do_hash(static_cast<void*>(val), str_size_);
        }
        return 0;
    }

    string_t get_value_string() {
        switch (type_id_) {
            case TypeId::BOOLEAN:
                if (value_.boolean) {
                    return "true";
                }
                return "false";
            case TypeId::INTEGER:
                return std::to_string(value_.integer);
            case TypeId::DECIMAL:
                return std::to_string(value_.decimal);
            case TypeId::CHAR: 
                return value_.char_;
            default:
                return string_t("INVALID VALUE");
        }
    }
private:
    union values {
        boolean_t boolean;
        integer_t integer;
        decimal_t decimal;
        char *char_; // string
    };
    
    values value_;
    TypeId type_id_;
    int str_size_ = -1;
};

} // namespace dawn
