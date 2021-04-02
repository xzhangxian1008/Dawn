#include "executors/union_executor.h"
#include "storage/page/table_page.h"

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

    initialize();
}

bool UnionExecutor::get_next(Tuple *tuple) {
    while (true) {
        while (children_[1]->get_next(right_child_tuple_)) {
            while (in_mem_pages_[in_mem_idx_]->get)
        }

        if (!load_another_batch()) {
            // all outer tuples have been consumed
            break;
        }
    }

    return false;
}

/** delete temporary pages and return memory to buffer pool manager */
void UnionExecutor::close() {
    BufferPoolManager *bpm = get_context()->get_buffer_pool_manager();
    bpm->return_threshold_page(threshold_pages_);
    threshold_pages_ = 0;

    spill_to_disk(false); // put all pages into avail_tmp_page_ to gather them together

    while (!avail_tmp_page_.empty()) {
        bpm->delete_page(avail_tmp_page_.front());
        avail_tmp_page_.pop_front();
    }

    children_[0]->close();
    children_[1]->close();
    delete output_schema_;
}

void UnionExecutor::initialize() {
    TablePage *left_tb_page = reinterpret_cast<TablePage*>(get_context()->get_buffer_pool_manager()->new_tmp_page());
    if (left_tb_page == nullptr) {
        FATAL("UnionExecutor Error: left_tb_page == nullptr");
    }

    in_mem_pages_.push_back(left_tb_page);
    RID rid;
    
    BufferPoolManager *bpm = get_context()->get_buffer_pool_manager();

    /**
     * FIXME Very inefficient!!!
     * Spill to the disk operation will copy all of the outer table's data
     */
    while (children_[0]->get_next(left_child_tuple_)) {
        while (!left_tb_page->insert_tuple(*left_child_tuple_, &rid)) {
            // enter here means the TablePage has no more space to insert a tuple
            if (in_mem_pages_.size() + 1 > (1/2) * threshold_pages_) {
                // trigger the spill to disk operation
                spill_to_disk(true);
            }

            // get a new page to insert the tuple
            left_tb_page = reinterpret_cast<TablePage*>(bpm->new_tmp_page());
            if (left_tb_page == nullptr) {
                FATAL("UnionExecutor Error: left_tb_page == nullptr");
            }

            in_mem_pages_.push_back(left_tb_page);
        }
    }
}

void UnionExecutor::spill_to_disk(bool is_dirty) {
    BufferPoolManager *bpm = get_context()->get_buffer_pool_manager();
    while (!in_mem_pages_.empty()) {
        TablePage *page = in_mem_pages_.front();
        bpm->unpin_page(page->get_page_id(), is_dirty);
        in_mem_pages_.pop_front();
        avail_tmp_page_.push_back(page->get_page_id());
    }
}

// void UnionExecutor::extract_from_disk() {
//     BufferPoolManager *bpm = get_context()->get_buffer_pool_manager();
//     while (in_mem_pages_.size() < (1/2) * threshold_pages_) {
//         Page *page = bpm->get_page(avail_tmp_page_.front());
//         avail_tmp_page_.pop_front();
//         in_mem_pages_.push_back(page);
//     }
// }

bool UnionExecutor::load_another_batch() {
    BufferPoolManager *bpm = get_context()->get_buffer_pool_manager();
    
    while (!in_mem_pages_.empty()) {
        TablePage *page = in_mem_pages_.front();
        bpm->delete_page(page->get_page_id());
        in_mem_pages_.pop_front();
    }

    while (in_mem_pages_.size() < (1/2) * threshold_pages_) {
        TablePage *page = reinterpret_cast<TablePage*>(bpm->get_page(avail_tmp_page_.front()));
        avail_tmp_page_.pop_front();
        in_mem_pages_.push_back(page);
    }

    if (in_mem_pages_.size() > 0) {
        in_mem_idx_ = 0;
        return true;
    }

    return false;
}

} // namespace dawn
