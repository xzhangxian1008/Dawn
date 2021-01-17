#pragma once

#include "data/values.h"
#include "util/config.h"

namespace dawn {
class Type;

extern Type *singleton[2];

// TODO VARCHAR
enum class TypeId : enum_size_t { INVALID = -1, BOOLEAN, INTEGER, DECIMAL, CHAR };

enum class CmpResult : enum_size_t { TRUE = 0, FALSE };

class Value;

class Type {
public:
    explicit Type() = default;
    virtual ~Type() = default;

    // compare operation
    virtual CmpResult cmp_less(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_less_and_eq(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_eq(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_greater_and_eq(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_greater(const Value &left, const Value &right) = 0;

    virtual Value minus(const Value &left, const Value &right) = 0;
    virtual Value add(const Value &left, const Value &right) = 0;
    virtual Value multiply(const Value &left, const Value &right) = 0;
    virtual Value divide(const Value &left, const Value &right) = 0;
    virtual Value min(const Value &left, const Value &right) = 0;
    virtual Value max(const Value &left, const Value &right) = 0;

    inline static Type* get_instance(TypeId type_id) { return singleton[(int)type_id]; }
    inline static size_t_ get_bool_size() { return sizeof(boolean_t); }
    inline static size_t_ get_integer_size() { return sizeof(integer_t); }
    inline static size_t_ get_decimal_size() { return sizeof(decimal_t); }

    static size_t_ get_type_size(TypeId type_id) {
        switch (type_id) {
            case TypeId::BOOLEAN:
                return sizeof(boolean_t);
            case TypeId::INTEGER:
                return sizeof(integer_t);
            case TypeId::DECIMAL:
                return sizeof(decimal_t);
            default:
                return -1;
        }
    }
};

string_t type_to_string(TypeId type_id);
    
} // namespace dawn
