#include "data/values.h"
#include "data/types.h"
#include "data/boolean.h"
#include "data/integer.h"

namespace dawn {

Type *singleton[2] = { new Boolean(), new Integer()};

Value::Value() : str_size_(-1) {
    type_id_ = TypeId::INVALID;
}
    
Value::Value(bool val) : str_size_(-1) {
    type_id_ = TypeId::BOOLEAN;
    value_.boolean = val;
}

Value::Value(__INT32_TYPE__ val) : str_size_(-1) {
    type_id_ = TypeId::INTEGER;
    value_.integer = val;
}

Value::Value(double val) : str_size_(-1) {
    type_id_ = TypeId::DECIMAL;
    value_.decimal = val;
}

Value::Value(char *val, int size) : str_size_(size) {
    type_id_ = TypeId::CHAR;
    value_.char_ = new char[size+1];
    memcpy(value_.char_, val, size);
    value_.char_[size] = '\0';
}

Value::Value(const string_t &val) : str_size_(val.length()) {
    type_id_ = TypeId::CHAR;
    value_.char_ = new char[str_size_+1];
    for (int i = 0; i < str_size_; i++)
        value_.char_[i] = val[i];
    value_.char_[str_size_] = '\0';
}

Value::~Value() {
    if (type_id_ == TypeId::CHAR)
        delete value_.char_;
}

} // namespace dawn
