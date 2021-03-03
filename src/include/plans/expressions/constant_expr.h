#pragma once

#include "plans/expressions/expr_abstr.h"
#include "data/values.h"

namespace dawn {

class ConstantExpression : public ExpressionAbstract {
public:
    ConstantExpression(const Value &val) : val_(val){}
    Value Evaluate(const Tuple *tuple, const Schema *schema) override { return val_; }
private:
    Value val_;
};

} // namespace dawn
