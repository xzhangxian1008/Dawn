#include "data/values.h"
#include "data/types.h"
#include "data/boolean.h"
#include "data/integer.h"
#include "data/decimal.h"
#include "data/char.h"

namespace dawn {

Type *singleton[4] = { new Boolean(), new Integer(), new Decimal(), new Char()};

Value::Value() {
    type_id_ = TypeId::INVALID;
}
    
Value::Value(boolean_t val) {
    type_id_ = TypeId::BOOLEAN;
    value_.boolean = val;
}

Value::Value(integer_t val) {
    type_id_ = TypeId::INTEGER;
    value_.integer = val;
}

Value::Value(decimal_t val) {
    type_id_ = TypeId::DECIMAL;
    value_.decimal = val;
}

Value::Value(char *val, int size) : str_size_(size) {
    type_id_ = TypeId::CHAR;
    value_.char_ = new char[size+1];
    memset(value_.char_, 0, size + 1);
    memcpy(value_.char_, val, size);
    value_.char_[size] = '\0'; // remind us that there needs a string end
}

Value::Value(const string_t &val) : str_size_(val.length()) {
    type_id_ = TypeId::CHAR;
    value_.char_ = new char[str_size_+1];
    memset(value_.char_, 0, str_size_ + 1);
    for (int i = 0; i < str_size_; i++)
        value_.char_[i] = val[i];
    value_.char_[str_size_] = '\0'; // remind us that there needs a string end
}

Value::Value(char *value, TypeId type_id, size_t_ str_size) : type_id_(type_id) {
    if (type_id_ == TypeId::CHAR) {
        str_size_ = str_size;
        value_.char_ = new char[str_size_+1];
        memset(value_.char_, 0, str_size_ + 1);
    }
    deserialize_from(value);
}

Value::~Value() {
    if (type_id_ == TypeId::CHAR)
        delete[] value_.char_;
}

} // namespace dawn
