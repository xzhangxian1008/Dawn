#pragma once

#include <string>
#include <string.h>

#include "data/types.h"
#include "util/config.h"

namespace dawn {
enum class TypeId;
class Type;

extern Type *singleton[4];

class Value {
public:
    Value();
    Value(bool val);
    Value(__INT32_TYPE__ val);
    Value(double val);
    Value(char *val, int size);
    Value(const string_t &val);
    Value(char *value, TypeId type_id);
    ~Value();

    void swap(Value &val) {
        std::swap(val.type_id_, type_id_);
        std::swap(val.value_, value_);
        std::swap(val.str_size_, str_size_);
    }

    // similar to shallow copy
    void load(const Value &val) {
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

    inline void serialize_to(char *storage) { singleton[(int)type_id_]->serialize_to(storage, (char*)(&value_)); }
    inline void deserialize_from(char *src) { singleton[(int)type_id_]->deserialize_from((char*)(&value_), src); }

private:
    union values {
        boolean_t boolean;
        integer_t integer;
        decimal_t decimal;
        char *char_;
    };
    
    values value_;
    TypeId type_id_;
    int str_size_;
};

} // namespace dawn
