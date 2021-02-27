#include "executors/proj_executor.h"

namespace dawn {

void ProjectionExecutor::open() {
    child_->open();
}

bool ProjectionExecutor::get_next(Tuple *tuple) {
    if(!child_->get_next(&next_tuple_))
        return false;

    std::vector<Value> vals;
    for (auto expr : exprs_)
        vals.push_back(expr->Evaluate(&next_tuple_, input_schema_));

    tuple->reconstruct(&vals, *output_schema_);
    return true;
}

void ProjectionExecutor::close() {
    child_->close();
}

} // namespace dawn
