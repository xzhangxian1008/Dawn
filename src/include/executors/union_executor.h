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
     * load all the outer table's tuples into memory, and put them on the disk
     * when the number ofallocated pages for outer table's tuples exceed the threshold
     */
    void initialize();

    /**
     * get next left child's tuple in the temporary pages
     * @return true: successfully false: no more tuples
     */
    bool get_next_left_tuple();

    /** spill the outer table's data which stay in the memory to the disk */
    void spill_to_disk(bool is_dirty);

    /** get a batch of outer table's data from the disk */
    // void extract_from_disk();

    /**
     * After traversing all the inner table's tuples, we should load another
     * batch of outer table's tuples from disk and delete all the temporary pages
     * that are current in the buffer pool manager.
     * 
     * FIXME Very inefficient!!!
     * This function will stop the thread for a long time because of the I/O. 
     * We may set two buffer. When one is used for concatenating with inner table's 
     * tuples, the other could be under the I/O operation so that we can keep cpu running.
     */
    bool load_another_batch();

    /** available temporary pages that store the left child's tuple */
    std::deque<page_id_t> avail_tmp_page_;

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
    std::deque<TablePage*> in_mem_pages_;

    /**
     * help to locate which page in the memory last time we visited
     * so that we can know where to get the next outer table's tuple.
     */
    int in_mem_idx_;

    /** a container to contain the left child's tuple so that we could reuse it repeatedly */
    Tuple *left_child_tuple_;

    /** a container to contain the right child's tuple so that we could reuse it repeatedly */
    Tuple *right_child_tuple_;
};

} // namespace dawn
