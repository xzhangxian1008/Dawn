#include "executors/proj_executor.h"

namespace dawn {

void ProjectionExecutor::open() {
    child_->open();

    /**
     * do aggregation at the beginning
     * 
     * In order to get the aggregation result first, we need to traverse the table,
     * close it and reopen it. If the child is a union executor, it means that we
     * will union the whole two table for twice, it's very inefficient!
     */
    if (is_aggregate_) {
        for (int i = 0; i < agg_num_; i++) 
            agg_vals_.push_back(Value(static_cast<integer_t>(0)));

        while (child_->get_next(&next_tuple_)) {
            for (int i = 0; i < agg_num_; i++)
                agg_vals_[i] = agg_exprs_[i]->evaluate(&next_tuple_, input_schema_);
        }
        child_->close();
        child_->open();
    }
}

bool ProjectionExecutor::get_next(Tuple *tuple) {
    if(!child_->get_next(&next_tuple_))
        return false;

    std::vector<Value> vals;
    for (auto expr : exprs_)
        vals.push_back(expr->evaluate(&next_tuple_, input_schema_));

    if (is_aggregate_) {
        for (int i = 0; i < agg_num_; i++)
            vals.push_back(agg_vals_[i]);
    }
    tuple->reconstruct(&vals, *output_schema_);
    return true;
}

void ProjectionExecutor::close() {
    child_->close();
}

} // namespace dawn
