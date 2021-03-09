#pragma once

#include "executors/executor_abstr.h"
#include "table/schema.h"
#include <deque>

namespace dawn {

class UnionExecutor : public ExecutorAbstract {
public:
    UnionExecutor(ExecutorContext *exec_ctx, std::vector<ExecutorAbstract*> children, std::vector<Schema*> child_schema)
    : ExecutorAbstract(exec_ctx), children_(children), child_schema_(child_schema) {}

    ~UnionExecutor() override { delete output_schema_; }

    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;

private:
    /** store left and right child, and see left child as inner table */
    std::vector<ExecutorAbstract*> children_;

    /** store left and right child's schema */
    std::vector<Schema*> child_schema_;

    Schema *output_schema_;

    /**
     * When the inner table is too large, we should spill some data to temporary pages on the disk.
     * This field is used for checking if we should trigger the spill to disk function.
     */
    size_t_ threshold_pages_;

    /** record how many pages have been used for the inner table */
    size_t_ page_usage_ = 0;

    /** store pages in memory */
    std::deque<Page*> in_mem_pages_;

    /** stored temporary pages in disk */
    std::deque<page_id_t> in_disk_pages_;
};

} // namespace dawn
