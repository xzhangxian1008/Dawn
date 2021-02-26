#pragma once

#include "plans/expressions/expr_abstr.h"
#include <vector>

namespace dawn {

class ColumnValueExpression : ExpressionAbstract {
public:
    ColumnValueExpression(offset_t col_idx) : col_idx_(col_idx) {}
    ~ColumnValueExpression() override {}
    Value Evaluate(const Tuple *tuple, const Schema *schema) override {
        
    }
private:
    offset_t col_idx_;
};

} // namespace dawn