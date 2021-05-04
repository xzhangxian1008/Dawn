#pragma once

#include "executors/executor_abstr.h"
#include "plans/expressions/expr_abstr.h"
#include "plans/expressions/comparison_expr.h"
#include "table/schema.h"
#include "util/config.h"

namespace dawn {

class SelectionExecutor : public ExecutorAbstract {
public:
    SelectionExecutor(ExecutorContext *exec_ctx, ExpressionAbstract *predicate, ExecutorAbstract *child, Schema *schema)
        : ExecutorAbstract(exec_ctx), predicate_(predicate), child_(child), schema_(schema), cmp_(true) {}
    ~SelectionExecutor() = default;

    DISALLOW_COPY_AND_MOVE(SelectionExecutor);

    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;
private:
    ExpressionAbstract *predicate_;
    ExecutorAbstract *child_;
    Schema *schema_;
    Value cmp_; // in avoid of the repeated constructor and deconstructor
};
    
} // namespace dawn