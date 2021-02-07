#pragma once

#include "manager/db_manager.h"
#include "buffer/buffer_pool_manager.h"
#include "table/tuple.h"

namespace dawn {

/**
 * directly get tuple
 * @param rid target tuple's position
 * @param tuple return tuple with this pointer
 */
inline op_code_t get_tuple_directly(const RID &rid, Tuple *tuple) {
    BufferPoolManager *bpm = db_ptr->get_buffer_pool_manager();
    TablePage *tb_page = reinterpret_cast<TablePage*>(bpm->get_page(rid.get_page_id()));
    if (tb_page == nullptr) {
        return TUPLE_NOT_FOUND;
    }

    tb_page->r_lock();
    if (tb_page->get_tuple(tuple, rid)) {
        tb_page->r_unlock();
        bpm->unpin_page(rid.get_page_id(), false);
        return OP_SUCCESS;
    }
    tb_page->r_unlock();
    bpm->unpin_page(rid.get_page_id(), false);
    return TUPLE_NOT_FOUND;
}

} // namespace dawn
