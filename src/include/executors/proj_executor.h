#pragma once

#include "executors/executor_abstr.h"
#include "plans/expressions/col_value_expr.h"
#include "table/schema.h"
#include "util/config.h"
#include "data/types.h"

namespace dawn {

template<typename T>
Value init_agg_min_val() {
    return Value(T INT32_MAX);
}

template<typename T>
Value init_agg_max_val() {
    return Value(T INT32_MIN);
}

template<typename T>
Value init_agg_sum_val() {
    return Value(static_cast<T>(0));
}

inline Value init_agg_cnt_val() {
    return Value(static_cast<integer_t>(0));
}

/**
 * For convenience, projection and aggregation function are put together in this ProjectionExecutor.
 * 
 * We put all the aggregation result at the end of the tuple when we need aggregation,
 * so that we can construct a tuple more convenient. output schema should follow this rule
 * when constructing
 */
class ProjectionExecutor : public ExecutorAbstract {
public:
    ProjectionExecutor(ExecutorContext *exec_ctx, ExecutorAbstract *child, std::vector<ExpressionAbstract*> exprs,
        Schema *input_schema, Schema *output_schema)
        : ExecutorAbstract(exec_ctx), child_(child), exprs_(exprs), input_schema_(input_schema),
         output_schema_(output_schema), is_aggregate_(false) {}

    // ProjectionExecutor(ExecutorContext *exec_ctx, ExecutorAbstract *child, std::vector<ExpressionAbstract*> exprs,
    //     Schema *input_schema, Schema *output_schema, std::vector<AggregationType> agg_type)
    //     : ExecutorAbstract(exec_ctx), child_(child), exprs_(exprs), input_schema_(input_schema),
    //      output_schema_(output_schema), is_aggregate_(true), agg_num_(agg_type.size()) {
        
    //     size_t_ input_sch_col_num = input_schema_->get_column_num();

    //     // initialize the Value of the aggregation
    //     for (int i = 0; i < agg_num_; i++) {
    //         TypeId type_id = output_schema_->get_column_type(input_sch_col_num + i);
    //         switch (agg_type[i]) {
    //             case AggregationType::CountAggregate:
    //                 if (type_id == TypeId::INTEGER) {

    //                 } else if ()
    //                 break;
    //             case AggregationType::SumAggregate:
    //                 break;
    //             case AggregationType::MinAggregate:
    //                 break;
    //             case AggregationType::MaxAggregate:
    //                 agg_vals_.emplace_back(init_agg_cnt_val());
    //                 agg_exprs_.emplace_back(AggregateExpression(agg_type[i], input_sch_col_num + i));
    //                 break;
    //             default:
    //                 LOG("Invalid AggregationType");
    //                 exit(-1);
    //         }
    //     }
    // }

    ~ProjectionExecutor() = default;

    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;
private:
    ExecutorAbstract *child_;
    std::vector<ExpressionAbstract*> exprs_; // get the columns from input tuple
    Tuple next_tuple_; // in avoid of the construction and deconstruction every time we call the get_next()
    Schema *input_schema_;
    Schema *output_schema_;

    // std::vector<AggregateExpression> agg_exprs_; // aggregate operation
    bool is_aggregate_;
    std::vector<Value> agg_vals_; // store the aggregation result
    int agg_num_;
};

} // namespace dawn