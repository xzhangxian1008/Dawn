#include "data/values.h"
#include "data/types.h"
#include "data/boolean.h"
#include "data/integer.h"
#include "data/decimal.h"
#include "data/char.h"

namespace dawn {

Type *singleton[4] = { new Boolean(), new Integer(), new Decimal(), new Char()};

Value::Value() {
    type_id_ = TypeId::kInvalid;
}
    
Value::Value(boolean_t val) {
    construct(val);
}

Value::Value(CmpResult val) {
    construct(val);
}

Value::Value(integer_t val) {
    construct(val);
}

Value::Value(decimal_t val) {
    construct(val);
}

Value::Value(const char* val) : str_len_(strlen(val)) {
    construct(val, str_len_);
}

Value::Value(const string_t &val) : str_len_(val.length()) {
    type_id_ = TypeId::kChar;
    value_.char_ = new char[str_len_+1];
    memset(value_.char_, 0, str_len_ + 1);
    for (int i = 0; i < str_len_; i++)
        value_.char_[i] = val[i];
    value_.char_[str_len_] = '\0'; // remind us that there needs a string end
}

Value::Value(char *value, TypeId type_id, size_t_ str_size) : type_id_(type_id), str_len_(str_size) {
    if (type_id_ == TypeId::kChar) {
        value_.char_ = new char[str_len_+1];
        memset(value_.char_, 0, str_len_ + 1);
    }
    deserialize_from(value);
}

Value::~Value() {
    if (type_id_ == TypeId::kChar)
        delete[] value_.char_;
}

} // namespace dawn
