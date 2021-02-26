#pragma once

#include "executors/executor_abstr.h"
#include "plans/expressions/col_value_expr.h"
#include "util/config.h"

namespace dawn {

class ProjectionExecutor : public ExecutorAbstract {
public:
    ProjectionExecutor(ExecutorContext *exec_ctx, ExecutorAbstract *child)
        : ExecutorAbstract(exec_ctx), child_(child) {}
    ~ProjectionExecutor() = default;
    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;
private:
    ExecutorAbstract *child_;

};

} // namespace dawn