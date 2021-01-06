#pragma once

#include "data/types.h"
#include "data/values.h"

namespace dawn {

class Boolean : public Type {
public:
    explicit Boolean() = default;
    ~Boolean() override {}

    CmpResult cmp_less(const Value &left, const Value &right) override { return CmpResult::FALSE; }
    CmpResult cmp_less_and_eq(const Value &left, const Value &right) override { return CmpResult::FALSE; }

    CmpResult cmp_eq(const Value &left, const Value &right) override {
        boolean_t left_val = left.get_value<boolean_t>();
        boolean_t right_val = right.get_value<boolean_t>();
        return (left_val == right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_greater_and_eq(const Value &left, const Value &right) override { return CmpResult::FALSE; }
    CmpResult cmp_greater(const Value &left, const Value &right) override { return CmpResult::FALSE; }

    Value minus(const Value &left, const Value &right) override { return Value(); }
    Value add(const Value &left, const Value &right) override { return Value(); }
    Value multiply(const Value &left, const Value &right) { return Value(); }
    Value divide(const Value &left, const Value &right) override { return Value(); }
    Value min(const Value &left, const Value &right) override { return Value(); }
    Value max(const Value &left, const Value &right) override { return Value(); }
};

} // namespace dawn