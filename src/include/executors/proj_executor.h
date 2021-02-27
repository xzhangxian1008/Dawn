#pragma once

#include "executors/executor_abstr.h"
#include "plans/expressions/col_value_expr.h"
#include "table/schema.h"
#include "util/config.h"

namespace dawn {

class ProjectionExecutor : public ExecutorAbstract {
public:
    ProjectionExecutor(ExecutorContext *exec_ctx, ExecutorAbstract *child, std::vector<ExpressionAbstract*> exprs,
        Schema *input_schema, Schema *output_schema)
        : ExecutorAbstract(exec_ctx), child_(child), exprs_(exprs), input_schema_(input_schema), output_schema_(output_schema) {}
    ~ProjectionExecutor() = default;
    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;
private:
    ExecutorAbstract *child_;
    std::vector<ExpressionAbstract*> exprs_;
    Tuple next_tuple_; // in avoid of the construction and deconstruction every time we call the get_next()
    Schema *input_schema_;
    Schema *output_schema_;
};

} // namespace dawn