#include "data/types.h"

namespace dawn {

class Char : public Type {
public:
    Char() = default;
    ~Char() override {}

    // TODO implement these cmp function
    CmpResult cmp_less(const Value &left, const Value &right) override { return CmpResult::kFalse; }
    CmpResult cmp_less_and_eq(const Value &left, const Value &right) override { return CmpResult::kFalse; }
    CmpResult cmp_eq(const Value &left, const Value &right) override { return CmpResult::kFalse; }
    CmpResult cmp_not_eq(const Value &left, const Value &right) override { return CmpResult::kFalse; }
    CmpResult cmp_greater_and_eq(const Value &left, const Value &right) override { return CmpResult::kFalse; }
    CmpResult cmp_greater(const Value &left, const Value &right) override { return CmpResult::kFalse; }
    Value minus(const Value &left, const Value &right) override { return Value(); }
    Value add(const Value &left, const Value &right) override { return Value(); }
    Value multiply(const Value &left, const Value &right) { return Value(); }
    Value divide(const Value &left, const Value &right) override { return Value(); }
    Value min(const Value &left, const Value &right) override { return Value(); }
    Value max(const Value &left, const Value &right) override { return Value(); }
    void serialize_to(char *dst, char *src) override { memcpy(dst, src, strlen(src)+1); }
    void deserialize_from(char *dst, char *src) override {memcpy(dst, src, strlen(src)+1); }
};

} // namespace dawn
