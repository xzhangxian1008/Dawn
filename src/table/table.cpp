#include "table/table.h"

namespace dawn {

Table::Table(BufferPoolManager *bpm, const page_id_t first_table_page_id, bool from_scratch) :
    bpm_(bpm), first_table_page_id_(first_table_page_id) {
    if (!from_scratch)
        return;
    
    Page *page = bpm_->get_page(first_table_page_id_);
    if (page == nullptr) {
        PRINT("ERROR! get *page nullptr");
        exit(-1);
    }
    TablePage *table_page = reinterpret_cast<TablePage*>(page);
    table_page->init(-1, -1);
    bpm_->unpin_page(first_table_page_id_, true);
}

} // namespace dawn
