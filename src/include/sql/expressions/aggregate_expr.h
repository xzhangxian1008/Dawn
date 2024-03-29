#pragma once

#include "plans/expressions/expr_abstr.h"
#include "data/types.h"

namespace dawn {

class AggregateExpression : public ExpressionAbstract {
public:
    AggregateExpression(AggregationType type, offset_t agg_idx) : type_(type), agg_idx_(agg_idx) {
        switch (type_) {
            case AggregationType::kCountAggregate: 
            case AggregationType::kSumAggregate: 
                val_ = Value(static_cast<integer_t>(0));
                break;
            case AggregationType::kMinAggregate:
                val_ = Value(static_cast<integer_t>(INT32_MAX));
                break;
            case AggregationType::kMaxAggregate:
                val_ = Value(static_cast<integer_t>(INT32_MIN));
                break;
            default:
                LOG("should not reach here");
                break;
        }
    }
    ~AggregateExpression() = default;
    Value evaluate(const Tuple *tuple, const Schema *schema) override {
        perform_aggregation(tuple, schema);
        return val_;
    }
private:
    void perform_aggregation(const Tuple *tuple, const Schema *schema) {
        switch (type_) {
            case AggregationType::kCountAggregate:  {
                ++val_;
                break;
            }
            case AggregationType::kSumAggregate: {
                val_ = ADD(val_.get_type_id(), val_, tuple->get_value(*schema, agg_idx_));
                break;
            }
            case AggregationType::kMinAggregate: {
                if (CMP_LESS(val_.get_type_id(), tuple->get_value(*schema, agg_idx_), val_) == CmpResult::kTrue)
                    val_ = tuple->get_value(*schema, agg_idx_);
                break;
            }
            case AggregationType::kMaxAggregate: {
                if (CMP_GREATER(val_.get_type_id(), tuple->get_value(*schema, agg_idx_), val_) == CmpResult::kTrue)
                    val_ = tuple->get_value(*schema, agg_idx_);
                break;
            }
            default:
                LOG("should not reach here");
                break;
        }
    }

    Value val_;
    offset_t agg_idx_;
    AggregationType type_;
};

} // namespace dawn
