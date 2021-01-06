#pragma once

#include <string>
#include <string.h>

#include "data/types.h"
#include "util/config.h"

namespace dawn {

class Value {
public:
    Value();
    Value(bool val);
    Value(__INT32_TYPE__ val);
    Value(double val);
    Value(char *val, int size);
    Value(const string_t &val);
    ~Value();

    TypeId get_type_id() const { return type_id_; }

    template<typename T>
    T* get_value() const {
        return reinterpret_cast<T*>(&value_);
    }

    int get_varchar_size() const { return varchar_size_; }

private:
    union values {
        bool boolean;
        __INT32_TYPE__ integer;
        double decimal;
        char *varchar;
    };
    
    values value_;
    TypeId type_id_;
    int varchar_size_;
};

} // namespace dawn
