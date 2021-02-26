#pragma once

#include <vector>
#include "executors/executor_context.h"
#include "table/tuple.h"

namespace dawn {

class ExecutorAbstract {
public:
    explicit ExecutorAbstract(ExecutorContext *exec_ctx) : exec_ctx_{exec_ctx} {}
    virtual ~ExecutorAbstract() = default;
    virtual void open() = 0;
    virtual bool get_next(Tuple *tuple) = 0;
    virtual void close() = 0;
    const ExecutorContext* get_context() const { return exec_ctx_; }
protected:
    ExecutorContext *exec_ctx_;
};

} // namespace dawn
