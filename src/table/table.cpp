#include "table/table.h"
#include "storage/page/link_hash_page.h"
#include <set>

namespace dawn {

Table::Table(BufferPoolManager *bpm, const page_id_t first_table_page_id, bool from_scratch) :
    bpm_(bpm), first_table_page_id_(first_table_page_id) {
    switch (LINK_HASH) {
        case LINK_HASH:
            insert_tuple_func = lk_ha_insert_tuple;
            mark_delete_func = lk_ha_mark_delete;
            apply_delete_func = lk_ha_apply_delete;
            rollback_delete_func = lk_ha_rollback_delete;
            get_tuple_func = lk_ha_get_tuple;
            update_tuple_func = lk_ha_update_tuple;
            break;
        case BP_TREE:
            break;
        default:
            break;
    }

    if (!from_scratch)
        return;
    
    Page *page = bpm_->get_page(first_table_page_id_);
    if (page == nullptr) {
        LOG("ERROR! get *page nullptr");
        exit(-1);
    }

    // always inialize with link hash so far
    switch (LINK_HASH) {
        case LINK_HASH: {
            LinkHashPage *lk_ha_page = reinterpret_cast<LinkHashPage*>(page);
            lk_ha_page->init();
            break;
        }
        case BP_TREE: {
            LOG("should not reach here");
            break;
        }
        default: {
            LOG("should not reach here");
            break;
        }
    }

    bpm_->unpin_page(first_table_page_id_, true);
}

void Table::delete_all_data() {
    std::set<page_id_t> deleted_pages;
    page_id_t next_page_id = first_table_page_id_;
    
    // get all page id first
    while (next_page_id != -1) {
        deleted_pages.insert(next_page_id);
        TablePage *table_page = reinterpret_cast<TablePage*>(bpm_->get_page(next_page_id));
        if (table_page == nullptr) {
            LOG("delete all data fail");
            return;
        }
        bpm_->unpin_page(next_page_id, false);
        next_page_id = table_page->get_next_page_id();
    }

    // delete pages from disk
    for (auto &page_id : deleted_pages)
        bpm_->delete_page(page_id);
}

bool Table::mark_delete(const Value &key_value, const TableSchema &tb_schema) {
    if (mark_delete_func(first_table_page_id_, key_value, tb_schema, bpm_) == OP_SUCCESS)
        return true;
    return false;
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

void Table::apply_delete(const Value &key_value, const TableSchema &tb_schema) {
    Tuple tuple;
    if (!get_tuple(key_value, &tuple, tb_schema)) {
        return;
    }

    apply_delete_func(first_table_page_id_, key_value, tb_schema, bpm_);
}

void Table::apply_delete(const RID &rid) {
    page_id_t page_id = rid.get_page_id();
    if (page_id < 0)
        return;
    
    TablePage *table_page = reinterpret_cast<TablePage*>(bpm_->get_page(page_id));
    if (table_page == nullptr)
        return;    
    table_page->w_lock();
    table_page->apply_delete(rid);
    table_page->w_unlock();
    bpm_->unpin_page(page_id, true);
}

void Table::rollback_delete(const Value &key_value, const TableSchema &tb_schema) {
    rollback_delete_func(first_table_page_id_, key_value, tb_schema, bpm_);
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
    op_code_t op_code = get_tuple_directly(rid, tuple, bpm_);
    if (op_code == OP_SUCCESS) {
        return true;
    }
    return false;
}

bool Table::get_tuple(const Value &key_value, Tuple *tuple, const TableSchema &tb_schema) {
    op_code_t op_code = get_tuple_func(first_table_page_id_, key_value, tuple, tb_schema, bpm_);
    if (op_code == OP_SUCCESS) {
        return true;
    }
    return false;
}

bool Table::insert_tuple(Tuple *tuple, const TableSchema &tb_schema) {
    op_code_t op_code = insert_tuple_func(first_table_page_id_, tuple, tb_schema, bpm_);
    if (op_code == OP_SUCCESS) {
        return true;
    }
    return false;
}

bool Table::update_tuple(Tuple *new_tuple, const RID &old_rid, const TableSchema &tb_schema) {
    op_code_t op_code = update_tuple_func(first_table_page_id_, new_tuple, old_rid, tb_schema, bpm_);
    if (op_code == OP_SUCCESS) {
        return true;
    }
    return false;
}

} // namespace dawn
