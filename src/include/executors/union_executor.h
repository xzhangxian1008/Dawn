#pragma once

#include "executors/executor_abstr.h"
#include "table/schema.h"
#include <deque>

namespace dawn {

class UnionExecutor : public ExecutorAbstract {
public:
    UnionExecutor(ExecutorContext *exec_ctx, std::vector<ExecutorAbstract*> children, std::vector<Schema*> child_schema)
    : ExecutorAbstract(exec_ctx), children_(children), child_schema_(child_schema) {
        left_child_tuple_ = new Tuple();
        right_child_tuple_ = new Tuple();
    }

    ~UnionExecutor() override {
        delete left_child_tuple_;
        delete right_child_tuple_;
    }

    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;

private:

    /**
     * load as many as tuples it can from the left child until
     * it can't apply for more temporary pages
     */
    void initialize();

    /**
     * get next left child's tuple in the temporary pages
     * @return true: successfully false: no more tuples
     */
    bool get_next_left_tuple();

    /** spill the outer table's data which stay in the memory to the disk */
    void spill_to_disk(bool is_dirtr);

    /** get a batch of outer table's data from the disk */
    void extract_from_disk();

    /** available temporary pages that store the left child's tuple */
    std::deque<page_id_t> avail_tmp_page_;

    /**
     * when all the right child's tuple have been traversed, it will be
     * set to true to tell the executor that it should load another
     * batch of tuples in the left child's table
     */
    bool load_ = false;

    /** store left and right child, and see left child as inner table */
    std::vector<ExecutorAbstract*> children_;

    /** store left and right child's schema */
    std::vector<Schema*> child_schema_;

    Schema *output_schema_;

    /**
     * When the outer table is too large, we should spill some data to temporary pages on the disk.
     * This field is used for checking if we should trigger the spill to disk function.
     * 
     * When the outer table exceeds 1/2 of the threshold_pages_, spill the rest of the data to the disk.
     */
    size_t_ threshold_pages_;

    /** store outer table's pages in memory */
    std::deque<Page*> in_mem_pages_;

    /** a container to contain the left child's tuple so that we could reuse it repeatedly */
    Tuple *left_child_tuple_;

    /** a container to contain the right child's tuple so that we could reuse it repeatedly */
    Tuple *right_child_tuple_;
};

} // namespace dawn
