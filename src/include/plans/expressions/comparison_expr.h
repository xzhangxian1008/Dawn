#pragma once

#include <vector>
#include "plans/expressions/expr_abstr.h"
#include "data/types.h"
#include "data/boolean.h"

namespace dawn {

/** ComparisonType represents the type of comparison that we want to perform. */
enum class ComparisonType { Equal, NotEqual, LessThan, LessThanOrEqual, GreaterThan, GreaterThanOrEqual };

// class ComparisonAbstract : public ExpressionAbstract {
// public:
//     ComparisonAbstract(std::vector<ExpressionAbstract*> children, ComparisonType cmp_type)
//         : children_(children), cmp_type_(cmp_type) {}
//     Value Evaluate(const Tuple *tuple, const Schema *schema) override {
//         Value lhs = children_[0]->Evaluate(tuple, schema);
//         Value rhs = children_[1]->Evaluate(tuple, schema);
//         return 
//     }
// private:
//     CmpResult peform_cmp(const Value &lhs, const Value &rhs) const {
//         switch (cmp_type_) {
//             case ComparisonType::Equal:
//                 return CMP_EQ(lhs.get_type_id(), lhs, rhs);
//             case ComparisonType::NotEqual:
//                 return CMP_NOT_EQ(lhs.get_type_id(), lhs, rhs);
//             case ComparisonType::LessThan:
//                 return CMP_LESS(lhs.get_type_id(), lhs, rhs);
//             case ComparisonType::LessThanOrEqual:
//                 return CMP_LESS_EQ(lhs.get_type_id(), lhs, rhs);
//             case ComparisonType::GreaterThan:
//                 return CMP_GREATER(lhs.get_type_id(), lhs, rhs);
//             case ComparisonType::GreaterThanOrEqual:
//                 return CMP_GREATER_EQ(lhs.get_type_id(), lhs, rhs);
//             default: {
//                 LOG("should not reach here");
//                 exit(-1);
//             }
//         }
//     }

//     std::vector<ExpressionAbstract*> children_;
//     ComparisonType cmp_type_;
// };

} // namespace dawn