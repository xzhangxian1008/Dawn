#include "executors/seq_scan_executor.h"

namespace dawn {

void SeqScanExecutor::open() {
    tb_iter_ = new LinkHashTableIter(table_->get_first_table_page_id(), get_context()->get_buffer_pool_manager());
}

bool SeqScanExecutor::get_next(Tuple *tuple) {
    if ((*tb_iter_)->get_rid().get_page_id() == INVALID_PAGE_ID)
        return false;
    
    *tuple = *(*tb_iter_);
    ++(*tb_iter_);
    return true;
}

void SeqScanExecutor::close() {}


} // namespace dawn
