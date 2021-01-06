#pragma once

#include "data/types.h"
#include "data/values.h"

namespace dawn {

class Integer : public Type {
public:
    Integer() = default;
    ~Integer() override {}

    CmpResult cmp_less(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return (left_val < right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_less_and_eq(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return (left_val <= right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_eq(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return (left_val == right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_greater_and_eq(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return (left_val >= right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    CmpResult cmp_greater(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return (left_val > right_val) ? CmpResult::TRUE : CmpResult::FALSE;
    }

    Value minus(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return Value(left_val - right_val);
    }

    Value add(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return Value(left_val + right_val);
    }

    Value multiply(const Value &left, const Value &right) {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return Value(left_val * right_val);
    }

    Value divide(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return Value(left_val / right_val);
    }

    Value min(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return std::min(left_val, right_val);
    }

    Value max(const Value &left, const Value &right) override {
        integet_t left_val = left.get_value<integet_t>();
        integet_t right_val = right.get_value<integet_t>();
        return std::max(left_val, right_val);
    }
};

} // namespace dawn