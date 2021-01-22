#pragma once

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "buffer/buffer_pool_manager.h"

namespace dawn {

/**
 * ATTENTION not care about transaction so far
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
    ~TablePage() override {}
    
    void init(const page_id_t prev_pgid, const page_id_t next_pgid);

    page_id_t get_next_page_id() {
        return *reinterpret_cast<page_id_t*>(get_data() + NEXT_PGID_OFFSET);
    }

    page_id_t get_table_page_id() {
        return *reinterpret_cast<page_id_t*>(get_data() + PAGE_ID_OFFSET);
    }

    page_id_t get_prev_page_id() {
        return *reinterpret_cast<page_id_t*>(get_data() + PREV_PGID_OFFSET);
    }

    void set_prev_page_id(page_id_t page_id) {
        memcpy(get_data() + PREV_PGID_OFFSET, &page_id, PGID_T_SIZE);
    }

    void set_next_page_id(page_id_t page_id) {
        memcpy(get_data() + NEXT_PGID_OFFSET, &page_id, PGID_T_SIZE);
    }

    // TODO find, delete, update, insert tuple

    
private:
    static const offset_t PREV_PGID_OFFSET = COM_PG_HEADER_SZ;
    static const offset_t NEXT_PGID_OFFSET = PREV_PGID_OFFSET + sizeof(page_id_t);
    static const offset_t FREE_SPACE_PTR_OFFSET = NEXT_PGID_OFFSET + sizeof(page_id_t);
    static const offset_t TUPLE_CNT_OFFSET = FREE_SPACE_PTR_OFFSET + sizeof(page_id_t) + TABLE_PAGE_RESERVED;
    static const offset_t FIRST_TUPLE_OFFSET = TUPLE_CNT_OFFSET + sizeof(size_t_);
    static const offset_t INVALID_FREE_SPACE_PTR = PAGE_SIZE;
    static const size_t_ TUPLE_RECORD_SZ = SIZE_T_SIZE + OFFSET_T_SIZE;

    // tuple is marked as deleted if top bit of the tuple size is set to 1
    static const uint32_t DELETE_MASK = (1U << (8 * sizeof(uint32_t) - 1));

    static bool is_deleted(size_t_ tuple_size) {
        return static_cast<bool>(tuple_size & DELETE_MASK);
    }

    static size_t_ set_deleted_flag(size_t_ tuple_size) {
        return static_cast<size_t_>(tuple_size | DELETE_MASK);
    }
    
    static size_t_ unset_deleted_flag(size_t_ tuple_size) {
        return static_cast<size_t_>(tuple_size & (~DELETE_MASK));
    }

    void set_tuple_size(offset_t slot_num, size_t_ tuple_size) {
        memcpy(get_data() + FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ + OFFSET_T_SIZE, &tuple_size, SIZE_T_SIZE);
    }

    size_t_ get_tuple_size(offset_t slot_num, size_t_ tuple_size) {
        return *reinterpret_cast<size_t_*>(get_data() + FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ + OFFSET_T_SIZE);
    }

    void set_tuple_count(size_t_ tuple_count) {
        memcpy(get_data() + TUPLE_CNT_OFFSET, &tuple_count, SIZE_T_SIZE);
    }

    size_t_ get_tuple_count() {
        return *reinterpret_cast<size_t_*>(get_data() + TUPLE_CNT_OFFSET);
    }

    offset_t get_free_space_pointer() {
        return *reinterpret_cast<offset_t*>(get_data() + FREE_SPACE_PTR_OFFSET);
    }

    size_t_ get_free_space() {
        return get_free_space_pointer() - (FIRST_TUPLE_OFFSET + get_tuple_count() * TUPLE_RECORD_SZ);
    }
};
    
} // namespace dawn