#include "data/values.h"
#include "data/types.h"
#include "data/boolean.h"
#include "data/integer.h"

namespace dawn {

Type *singleton[2] = { new Boolean(), new Integer()};

Value::Value() : varchar_size_(-1) {
    type_id_ = TypeId::INVALID;
}
    
Value::Value(bool val) : varchar_size_(-1) {
    type_id_ = TypeId::BOOLEAN;
    value_.boolean = val;
}

Value::Value(__INT32_TYPE__ val) : varchar_size_(-1) {
    type_id_ = TypeId::INTEGER;
    value_.integer = val;
}

Value::Value(double val) : varchar_size_(-1) {
    type_id_ = TypeId::DECIMAL;
    value_.decimal = val;
}

Value::Value(char *val, int size) : varchar_size_(size) {
    type_id_ = TypeId::VARCHAR;
    value_.varchar = new char[size+1];
    memcpy(value_.varchar, val, size);
    value_.varchar[size] = '\0';
}

Value::Value(const string_t &val) : varchar_size_(val.length()) {
    type_id_ = TypeId::VARCHAR;
    value_.varchar = new char[varchar_size_+1];
    for (int i = 0; i < varchar_size_; i++)
        value_.varchar[i] = val[i];
    value_.varchar[varchar_size_] = '\0';
}

Value::~Value() {
    if (type_id_ == TypeId::VARCHAR)
        delete value_.varchar;
}

} // namespace dawn
