#pragma once

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "buffer/buffer_pool_manager.h"

namespace dawn {

/**
 * TablePage layout:
 * ------------------------------------------------------------------------
 * |                          common header (64)                          |
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 * | previous page id (4) | next page id (4) |   free space pointer (4)   |
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 * |                            reserved (64)                             |
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 * | tuple count (4) | tuple offset 1 (4) | tuple size 1 (4) |    .....   |
 * ------------------------------------------------------------------------
 * -----------------------------------------------
 * |     .....     | Tuple 3 | Tuple 2 | Tuple 1 |
 * -----------------------------------------------
 *                 ^
 *                 free space pointer
 */
class TablePage : public Page {
public:
    void init(const page_id_t prev_pgid, const page_id_t next_pgid);
    page_id_t get_next_page_id() { return *reinterpret_cast<page_id_t*>(get_data() + NEXT_PGID_OFFSET); }
    // TODO many many opration
    
private:
    static const offset_t PREV_PGID_OFFSET = COM_PG_HEADER_SZ;
    static const offset_t NEXT_PGID_OFFSET = PREV_PGID_OFFSET + sizeof(page_id_t);
    static const offset_t FREE_SPACE_PTR_OFFSET = NEXT_PGID_OFFSET + sizeof(page_id_t);
    static const offset_t TUPLE_CNT_OFFSET = FREE_SPACE_PTR_OFFSET + sizeof(page_id_t) + TABLE_PAGE_RESERVED;
    static const offset_t FIRST_TUPLE_OFFSET = TUPLE_CNT_OFFSET + sizeof(size_t_);
};
    
} // namespace dawn