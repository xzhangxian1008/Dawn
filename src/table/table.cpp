#include "table/table.h"
#include <set>

namespace dawn {

Table::Table(BufferPoolManager *bpm, const page_id_t first_table_page_id, bool from_scratch) :
    bpm_(bpm), first_table_page_id_(first_table_page_id) {
    if (!from_scratch)
        return;
    
    Page *page = bpm_->get_page(first_table_page_id_);
    if (page == nullptr) {
        LOG("ERROR! get *page nullptr");
        exit(-1);
    }
    TablePage *table_page = reinterpret_cast<TablePage*>(page);
    table_page->init(-1, -1);
    bpm_->unpin_page(first_table_page_id_, true);
}

void Table::delete_all_data() {
    std::set<page_id_t> deleted_pages;
    page_id_t next_page_id = first_table_page_id_;
    
    // get all page id first
    while (next_page_id != -1) {
        deleted_pages.insert(next_page_id);
        TablePage *table_page = reinterpret_cast<TablePage*>(bpm_->get_page(next_page_id));
        bpm_->unpin_page(next_page_id, false);
        next_page_id = table_page->get_next_page_id();
    }

    // delete pages from disk
    for (auto &page_id : deleted_pages)
        bpm_->delete_page(page_id);
}

} // namespace dawn
