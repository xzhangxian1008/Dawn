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

bool Table::mark_delete(const RID &rid) {
    page_id_t page_id = rid.get_page_id();
    if (page_id < 0)
        return false;
    
    TablePage *table_page = reinterpret_cast<TablePage*>(bpm_->get_page(page_id));
    table_page->w_lock();
    bool ok = table_page->mark_delete(rid);
    table_page->w_unlock();
    bpm_->unpin_page(page_id, true);
    return ok;
}

void Table::apply_delete(const RID &rid) {
    page_id_t page_id = rid.get_page_id();
    if (page_id < 0)
        return;
    
    TablePage *table_page = reinterpret_cast<TablePage*>(bpm_->get_page(page_id));
    table_page->w_lock();
    table_page->apply_delete(rid);
    table_page->w_unlock();
    bpm_->unpin_page(page_id, true);
}

void Table::rollback_delete(const RID &rid) {
    page_id_t page_id = rid.get_page_id();
    if (page_id < 0)
        return;
    
    TablePage *table_page = reinterpret_cast<TablePage*>(bpm_->get_page(page_id));
    table_page->w_lock();
    table_page->rollback_delete(rid);
    table_page->w_unlock();
    bpm_->unpin_page(page_id, true);
}

bool Table::get_tuple(Tuple *tuple, const RID &rid) {
    page_id_t page_id = rid.get_page_id();
    if (page_id < 0)
        return;
    
    TablePage *table_page = reinterpret_cast<TablePage*>(bpm_->get_page(page_id));
    table_page->w_lock();
    bool ok = table_page->get_tuple(tuple, rid);
    table_page->w_unlock();
    bpm_->unpin_page(page_id, false);
}

bool Table::insert_tuple(const Tuple &tuple, RID *rid) {
    page_id_t inserted_page_id = first_table_page_id_;
    TablePage *table_page = reinterpret_cast<TablePage*>(bpm_->get_page(inserted_page_id));

    // TODO there may be no more space in disk, handle this
    while (table_page->insert_tuple(tuple, rid)) {
        page_id_t next_page_id = table_page->get_next_page_id();
        if (next_page_id == INVALID_PAGE_ID) {
            
        }
    }
    bpm_->unpin_page(inserted_page_id, true);
}

} // namespace dawn
