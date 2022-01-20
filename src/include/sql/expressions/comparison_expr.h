#pragma once

#include <vector>
#include "sql/expressions/expr_abstr.h"
#include "data/types.h"
#include "data/boolean.h"

namespace dawn {

class ComparisonExpression : public ExpressionAbstract {
public:
    ComparisonExpression(std::vector<ExpressionAbstract*> children, ComparisonType cmp_type)
        : children_(children), cmp_type_(cmp_type) {}
    ~ComparisonExpression() = default;
    Value evaluate(const Tuple *tuple, const Schema *schema) override {
        Value lhs = children_[0]->evaluate(tuple, schema);
        Value rhs = children_[1]->evaluate(tuple, schema);
        return perform_cmp(lhs, rhs);
    }
private:
    CmpResult perform_cmp(const Value &lhs, const Value &rhs) const {
        switch (cmp_type_) {
            case ComparisonType::kEqual:
                return CMP_EQ(lhs.get_type_id(), lhs, rhs);
            case ComparisonType::kNotEqual:
                return CMP_NOT_EQ(lhs.get_type_id(), lhs, rhs);
            case ComparisonType::kLessThan:
                return CMP_LESS(lhs.get_type_id(), lhs, rhs);
            case ComparisonType::kLessThanOrEqual:
                return CMP_LESS_EQ(lhs.get_type_id(), lhs, rhs);
            case ComparisonType::kGreaterThan:
                return CMP_GREATER(lhs.get_type_id(), lhs, rhs);
            case ComparisonType::kGreaterThanOrEqual:
                return CMP_GREATER_EQ(lhs.get_type_id(), lhs, rhs);
            default: {
                LOG("should not reach here");
                exit(-1);
            }
        }
    }

    std::vector<ExpressionAbstract*> children_;
    ComparisonType cmp_type_;
};

} // namespace dawn
