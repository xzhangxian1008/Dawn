#pragma once

#include "executors/executor_abstr.h"
#include "plans/expressions/col_value_expr.h"
#include "table/schema.h"
#include "util/config.h"

namespace dawn {

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

    ProjectionExecutor(ExecutorContext *exec_ctx, ExecutorAbstract *child, std::vector<ExpressionAbstract*> exprs,
        Schema *input_schema, Schema *output_schema, std::vector<ExpressionAbstract*> agg_exprs)
        : ExecutorAbstract(exec_ctx), child_(child), exprs_(exprs), input_schema_(input_schema),
         output_schema_(output_schema), agg_exprs_(agg_exprs), is_aggregate_(true), agg_num_(agg_exprs_.size()) {}

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

    std::vector<ExpressionAbstract*> agg_exprs_; // aggregate operation
    bool is_aggregate_;
    std::vector<Value> agg_vals_; // store the aggregation result
    int agg_num_;
};

} // namespace dawn