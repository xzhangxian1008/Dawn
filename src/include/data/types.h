#pragma once

#include "data/values.h"

namespace dawn {

enum class TypeId { INVALID = -1, BOOLEAN, INTEGER, DECIMAL, VARCHAR };

enum class CmpResult { TRUE = 0, FALSE };

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

    virtual void swap(Value &left, Value &right) = 0;

    virtual Value minus(const Value &left, const Value &right) = 0;
    virtual Value add(const Value &left, const Value &right) = 0;
    virtual Value multiply(const Value &left, const Value &right) = 0;
    virtual Value divide(const Value &left, const Value &right) = 0;
    virtual Value min(const Value &left, const Value &right) = 0;
    virtual Value max(const Value &left, const Value &right) = 0;

private:

};
    
} // namespace dawn
