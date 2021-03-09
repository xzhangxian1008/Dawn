#include "executors/union_executor.h"

namespace dawn {

void UnionExecutor::open() {
    // initialize the threshold_pages_
    threshold_pages_ = get_context()->get_buffer_pool_manager()->get_threshold_page();

    // construct the output schema according to the child_schema
    std::vector<Column> left_child_col = child_schema_[0]->get_columns();
    std::vector<Column> right_child_col = child_schema_[1]->get_columns();

    // concatenate the left and right columns
    for (auto &col : right_child_col)
        left_child_col.push_back(col);
    
    output_schema_ = new Schema(left_child_col);
}

bool UnionExecutor::get_next(Tuple *tuple) {
    return true;
}

/** delete temporary pages and return and return memory to buffer pool manager */
void UnionExecutor::close() {
    

    get_context()->get_buffer_pool_manager()->return_threshold_page(threshold_pages_);
    threshold_pages_ = 0;

}

} // namespace dawn
