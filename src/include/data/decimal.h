#pragma once

#include "data/types.h"

namespace dawn {

class Decimal : public Type {
public:
    Decimal() = default;
    ~Decimal() override {}

    CmpResult cmp_less(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return (left_val < right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_less_and_eq(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return (left_val <= right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_eq(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return (left_val == right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_greater_and_eq(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return (left_val >= right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_greater(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return (left_val > right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    Value minus(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return Value(left_val - right_val);
    }

    Value add(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return Value(left_val + right_val);
    }

    Value multiply(const Value &left, const Value &right) {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return Value(left_val * right_val);
    }

    Value divide(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return Value(left_val / right_val);
    }

    Value min(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return std::min(left_val, right_val);
    }

    Value max(const Value &left, const Value &right) override {
        decimal_t left_val = left.get_value<decimal_t>();
        decimal_t right_val = right.get_value<decimal_t>();
        return std::max(left_val, right_val);
    }

    void serialize_to(char *dst, char *src) override { memcpy(dst, src, DECIMAL_T_SIZE); }
    void deserialize_from(char *dst, char *src) override { memcpy(dst, src, DECIMAL_T_SIZE); }
};

} // namespace dawn