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

/**
 * WARNING: DO NOT IMPLEMENT THE SHALLOW COPY!
 * Deconstructor always deletes the pointer when TypeId is kChar.
 */
class Value {
public:
    Value();
    Value(boolean_t val);
    Value(CmpResult val);
    Value(integer_t val);
    Value(decimal_t val);
    Value(const char* val);
    Value(const string_t &val);
    Value(char *value, TypeId type_id, size_t_ str_size = -1);
    ~Value();

    // TODO: construct should not be accessed in outside of the Value, or check TypeId before in the function

    void construct(boolean_t val) {
        type_id_ = TypeId::kBoolean;
        value_.boolean = val;
    }

    void construct(CmpResult val) {
        type_id_ = TypeId::kBoolean;
        if (val == CmpResult::kTrue)
            value_.boolean = true;
        else
            value_.boolean = false;
    }

    void construct(integer_t val) {
        type_id_ = TypeId::kInteger;
        value_.integer = val;
    }

    void construct(decimal_t val) {
        type_id_ = TypeId::kDecimal;
        value_.decimal = val;
    }

    void construct(const char* val, int size) {
        type_id_ = TypeId::kChar;
        value_.char_ = new char[size+1];
        memset(value_.char_, 0, size + 1);
        memcpy(value_.char_, val, size);
        value_.char_[size] = '\0'; // remind us that there needs a string end
    }

    // deep copy
    Value(const Value &value) {
        this->type_id_ = value.type_id_;
        this->str_len_ = value.str_len_;
        
        if (value.type_id_ != TypeId::kChar) {
            this->value_ = value.value_;
        } else {
            this->value_.char_ = new char[str_len_+1];
            memset(this->value_.char_, 0, str_len_ + 1);
            memcpy(this->value_.char_, value.value_.char_, str_len_ + 1);
        }
    }

    // deep copy
    Value& operator=(const Value &value) {
        if (this->type_id_ == TypeId::kChar) {
            delete[] value_.char_;
        }

        this->type_id_ = value.type_id_;
        this->str_len_ = value.str_len_;

        if (value.type_id_ != TypeId::kChar) {
            this->value_ = value.value_;
        } else {
            this->value_.char_ = new char[str_len_+1];
            memset(this->value_.char_, 0, str_len_ + 1);
            memcpy(this->value_.char_, value.value_.char_, str_len_ + 1);
        }

        return *this;
    }

    bool operator==(const Value &value) const {
        if (this->type_id_ != value.type_id_)
            return false;
            
        bool ok = true;
        switch (value.type_id_) {
            case TypeId::kInteger:
                if (this->value_.integer != value.value_.integer)
                    ok = false;
                break;
            case TypeId::kBoolean:
                if (this->value_.boolean != value.value_.boolean)
                    ok = false;
                break;
            case TypeId::kDecimal:
                if (this->value_.decimal != value.value_.decimal)
                    ok = false;
                break;
            case TypeId::kChar:
                if (string_t(this->value_.char_) != string_t(value.value_.char_)) 
                    ok = false;
                break;
            default:
                ok = false;
                break;
        }
        return ok;
    }

    bool operator!=(const Value &value) const {
        if (this->type_id_ != value.type_id_)
            return false;
            
        bool ok = true;
        switch (value.type_id_) {
            case TypeId::kInteger:
                if (this->value_.integer == value.value_.integer)
                    ok = false;
                break;
            case TypeId::kBoolean:
                if (this->value_.boolean == value.value_.boolean)
                    ok = false;
                break;
            case TypeId::kDecimal:
                if (this->value_.decimal == value.value_.decimal)
                    ok = false;
                break;
            case TypeId::kChar:
                if (string_t(this->value_.char_) == string_t(value.value_.char_)) 
                    ok = false;
                break;
            default:
                ok = false;
                break;
        }
        return ok;
    }

    bool operator<(const Value &value) const {
        // Comparison between different type of value is ub(undefined behaviour)
        if (this->type_id_ != value.type_id_)
            return false;
        
        switch (type_id_) {
            case TypeId::kBoolean: {
                return false; // illegal
            }
            case TypeId::kInteger: {
                return this->value_.integer < value.value_.integer;
            }
            case TypeId::kDecimal: {
                return this->value_.decimal < value.value_.decimal;
            }
            case TypeId::kChar: {
                string_t s1(value_.char_);
                string_t s2(value.value_.char_);
                if (s1.compare(s2) < 0)
                    return true;
                return false;
            }
            case TypeId::kInvalid: {
                return false;
            }
            default:
                return false;
        }
    }

    Value& operator++() {
        if (type_id_ == TypeId::kInteger) {
            value_.integer++;
        } else if (type_id_ == TypeId::kDecimal) {
            value_.decimal++;
        }
        return *this;
    }

    void swap(Value &val) {
        std::swap(val.type_id_, type_id_);
        std::swap(val.value_, value_);
        std::swap(val.str_len_, str_len_);
    }

    TypeId get_type_id() const { return type_id_; }

    template<typename T>
    T get_value() const {
        return *reinterpret_cast<const T*>(&value_);
    }

    int get_char_size() const { return str_len_; }

    // string should end with '\0'
    inline void serialize_to(char *storage) {
        if (type_id_ == TypeId::kChar) {
            singleton[static_cast<int>(type_id_)]->serialize_to(storage, value_.char_);
            return;
        }
        singleton[static_cast<int>(type_id_)]->serialize_to(storage, (char*)(&value_));
    }

    inline void deserialize_from(char *src) {
        if (type_id_ == TypeId::kChar) {
            singleton[static_cast<int>(type_id_)]->deserialize_from(value_.char_, src);
            return;
        }
        singleton[static_cast<int>(type_id_)]->deserialize_from((char*)(&value_), src);
    }

    // TODO: test it!
    hash_t get_hash_value() const {
        switch (type_id_) {
            case TypeId::kBoolean: {
                boolean_t *val = const_cast<boolean_t*>(&(value_.boolean));
                return do_hash(static_cast<void*>(val), BOOLEAN_T_SIZE);
            }
            case TypeId::kInteger: {
                integer_t *val = const_cast<integer_t*>(&(value_.integer));
                return do_hash(static_cast<void*>(val), INTEGER_T_SIZE);
            }
            case TypeId::kDecimal: {
                decimal_t *val = const_cast<decimal_t*>(&(value_.decimal));
                return do_hash(static_cast<void*>(val), DECIMAL_T_SIZE);
            }
            case TypeId::kChar: {
                char *val = const_cast<char*>(value_.char_);
                return do_hash(static_cast<void*>(val), str_len_);
            }
            case TypeId::kInvalid: {
                return 0;
            }
            default:
                return 0;
        }
        return 0;
    }

    string_t to_string() const {
        switch (type_id_) {
            case TypeId::kBoolean:
                if (value_.boolean) {
                    return "true";
                }
                return "false";
            case TypeId::kInteger:
                return std::to_string(value_.integer);
            case TypeId::kDecimal:
                return std::to_string(value_.decimal);
            case TypeId::kChar: 
                return value_.char_;
            default:
                return string_t("kInvalid VALUE");
        }
    }
private:
    union {
        boolean_t boolean;
        integer_t integer;
        decimal_t decimal;
        char *char_; // string
    } value_;
    
    TypeId type_id_;
    int str_len_ = -1;
};

} // namespace dawn
