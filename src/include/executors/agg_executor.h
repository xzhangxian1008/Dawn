#pragma once

#include "executors/executor_abstr.h"
#include "table/schema.h"

namespace dawn {

class AggregateExecutor : public ExecutorAbstract {
public:
    AggregateExecutor(ExecutorContext *ctx, std::vector<ExecutorAbstract*>predicates, Schema *input_schema, Schema *output_schema)
        : ExecutorAbstract(ctx), predicates_(predicates), input_schema_(input_schema), output_schema_(output_schema) {}
    ~AggregateExecutor() = default;
    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;
private:
    Schema *input_schema_;
    Schema *output_schema_;
    std::vector<ExecutorAbstract*> predicates_;
    ExecutorAbstract *child_;
};

} // namespace dawn