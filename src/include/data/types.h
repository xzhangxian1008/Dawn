#pragma once

#include "util/config.h"

namespace dawn {
class Type;

extern Type *singleton[4];

// TODO VARCHAR
enum class TypeId : enum_size_t { kInvalid = -1, kBoolean, kInteger, kDecimal, kChar };

enum class CmpResult : enum_size_t { kTrue = 0, kFalse };

/** ComparisonType represents the type of comparison that we want to perform. */
enum class ComparisonType { kEqual, kNotEqual, kLessThan, kLessThanOrEqual, kGreaterThan, kGreaterThanOrEqual };

/** AggregationType enumerates all the possible aggregation functions in our system. */
enum class AggregationType : enum_size_t { kCountAggregate, kSumAggregate, kMinAggregate, kMaxAggregate };

class Value;

class Type {
public:
    explicit Type() = default;
    virtual ~Type() = default;

    // compare operation
    virtual CmpResult cmp_less(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_less_and_eq(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_eq(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_not_eq(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_greater_and_eq(const Value &left, const Value &right) = 0;
    virtual CmpResult cmp_greater(const Value &left, const Value &right) = 0;

    virtual Value minus(const Value &left, const Value &right) = 0;
    virtual Value add(const Value &left, const Value &right) = 0;
    virtual Value multiply(const Value &left, const Value &right) = 0;
    virtual Value divide(const Value &left, const Value &right) = 0;
    virtual Value min(const Value &left, const Value &right) = 0;
    virtual Value max(const Value &left, const Value &right) = 0;

    virtual void serialize_to(char *dst, char *src) = 0;
    virtual void deserialize_from(char *dst, char *src) = 0;

    inline static Type* get_instance(TypeId type_id) { return singleton[(int)type_id]; }
    inline static size_t_ get_bool_size() { return BOOLEAN_T_SIZE; }
    inline static size_t_ get_integer_size() { return INTEGER_T_SIZE; }
    inline static size_t_ get_decimal_size() { return DECIMAL_T_SIZE; }

    static size_t_ get_type_size(TypeId type_id) {
        switch (type_id) {
            case TypeId::kBoolean:
                return BOOLEAN_T_SIZE;
            case TypeId::kInteger:
                return INTEGER_T_SIZE;
            case TypeId::kDecimal:
                return DECIMAL_T_SIZE;
            default:
                return -1;
        }
    }
};

string_t type_to_string(TypeId type_id);
    
} // namespace dawn
