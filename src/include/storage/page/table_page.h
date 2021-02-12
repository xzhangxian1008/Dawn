#pragma once

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "buffer/buffer_pool_manager.h"
#include "table/tuple.h"
#include "table/rid.h"

namespace dawn {

/**
 * WARNING DO NOT ADD ANY DATA MEMBER!
 * 
 * ATTENTION not care about transaction so far
 * 
 * concurrency control should be carried out by TablePage's caller
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
    ~TablePage() {}

    DISALLOW_COPY_AND_MOVE(TablePage);
    
    void init(const page_id_t prev_pgid, const page_id_t next_pgid);

    inline page_id_t get_next_page_id() {
        return *reinterpret_cast<page_id_t*>(get_data() + NEXT_PGID_OFFSET);
    }
    
    inline page_id_t get_prev_page_id() {
        return *reinterpret_cast<page_id_t*>(get_data() + PREV_PGID_OFFSET);
    }

    inline page_id_t get_table_page_id() const {
        return get_page_id();
    }

    inline void set_prev_page_id(page_id_t page_id) {
        memcpy(get_data() + PREV_PGID_OFFSET, &page_id, PGID_T_SIZE);
    }

    inline void set_next_page_id(page_id_t page_id) {
        memcpy(get_data() + NEXT_PGID_OFFSET, &page_id, PGID_T_SIZE);
    }

    bool insert_tuple(const Tuple &tuple, RID *rid);

    /**
     * not delete the tuple immediately, just mark it.
     */
    bool mark_delete(const RID &rid);

    /**
     * unset the tuple's DELETE_MASK
     */
    void rollback_delete(const RID &rid);

    void apply_delete(const RID &rid);

    /**
     * we suppose that all the table's tuples have the same size
     */
    bool update_tuple(const Tuple &new_tuple, const RID &rid);

    bool get_tuple(Tuple *tuple, const RID &rid) const;
    
    /**
     * @return true: get successfully, false: get unsuccessfully
     */
    bool get_next_tuple_rid(const RID &cur_rid, RID *next_rid) const;

    /**
     * @return true: get successfully, false: get unsuccessfully
     */
    bool get_the_first_tuple(Tuple *tuple) const;

    /**
     * @return number of the existing tuples
     */
    size_t_ get_stored_tuple_cnt() const;
    
    inline static size_t_ get_tuple_record_sz() { return TUPLE_RECORD_SZ; }

    inline static size_t_ get_tp_load_data_space() { return LOAD_DATA_SPACE; }
private:
    static const offset_t START_OFFSET = COM_PG_HEADER_SZ;
    static const offset_t PREV_PGID_OFFSET = START_OFFSET;
    static const offset_t NEXT_PGID_OFFSET = PREV_PGID_OFFSET + PGID_T_SIZE;
    static const offset_t FREE_SPACE_PTR_OFFSET = NEXT_PGID_OFFSET + PGID_T_SIZE;
    static const offset_t TUPLE_CNT_OFFSET = FREE_SPACE_PTR_OFFSET + PGID_T_SIZE + TABLE_PAGE_RESERVED;
    static const offset_t FIRST_TUPLE_OFFSET = TUPLE_CNT_OFFSET + SIZE_T_SIZE;
    static const offset_t INVALID_FREE_SPACE_PTR = PAGE_SIZE;
    static const size_t_ TUPLE_RECORD_SZ = SIZE_T_SIZE + OFFSET_T_SIZE;

    // describe how large space could be used to store tuple records and tuples' data in a TablePage
    static const size_t_ LOAD_DATA_SPACE = PAGE_SIZE - COM_PG_HEADER_SZ - 2 * PGID_T_SIZE - OFFSET_T_SIZE - SIZE_T_SIZE - TABLE_PAGE_RESERVED;

    // tuple is marked as deleted if top bit of the tuple size is set to 1
    static const uint32_t DELETE_MASK = (1U << (8 * sizeof(uint32_t) - 1));

    inline static bool is_deleted(size_t_ tuple_size) {
        return static_cast<bool>(tuple_size & DELETE_MASK);
    }

    inline static size_t_ set_deleted_flag(size_t_ tuple_size) {
        return static_cast<size_t_>(tuple_size | DELETE_MASK);
    }
    
    inline static size_t_ unset_deleted_flag(size_t_ tuple_size) {
        return static_cast<size_t_>(tuple_size & (~DELETE_MASK));
    }

    /**
     * the returned value may be overestimated, because some slots may be empty
     */
    inline size_t_ get_tuple_count() const {
        return *reinterpret_cast<size_t_*>(get_data() + TUPLE_CNT_OFFSET);
    }

    inline void set_tuple_size(offset_t slot_num, size_t_ tuple_size) {
        memcpy(get_data() + FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ + OFFSET_T_SIZE, &tuple_size, SIZE_T_SIZE);
    }

    inline size_t_ get_tuple_size(offset_t slot_num) const {
        return *reinterpret_cast<size_t_*>(get_data() + FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ + OFFSET_T_SIZE);
    }

    inline void set_tuple_offset(offset_t slot_num, offset_t offset) {
        *reinterpret_cast<offset_t*>(get_data() + FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ) = offset;
    }

    /** slot is empty if the offset is equal to 0 */
    inline offset_t get_tuple_offset(offset_t slot_num) const {
        return *reinterpret_cast<offset_t*>(get_data() + FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ);
    }

    inline void set_tuple_count(size_t_ tuple_count) {
        memcpy(get_data() + TUPLE_CNT_OFFSET, &tuple_count, SIZE_T_SIZE);
    }

    inline offset_t get_free_space_pointer() const {
        return *reinterpret_cast<offset_t*>(get_data() + FREE_SPACE_PTR_OFFSET);
    }

    inline size_t_ get_free_space() {
        return get_free_space_pointer() - (FIRST_TUPLE_OFFSET + get_tuple_count() * TUPLE_RECORD_SZ);
    }

    inline void set_free_space_pointer(offset_t fsp_offset) {
        *reinterpret_cast<offset_t*>(get_data() + FREE_SPACE_PTR_OFFSET) = fsp_offset;
    }

    inline void insert_tuple_record(offset_t slot_num, offset_t offset, size_t_ tuple_size) {
        *reinterpret_cast<offset_t*>(get_data() + FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ) = offset;
        *reinterpret_cast<offset_t*>(get_data() + FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ + OFFSET_T_SIZE) = tuple_size;
    }

    inline void update_tuple_record(offset_t pos, offset_t offset, size_t_ tuple_size) {
        *reinterpret_cast<offset_t*>(get_data() + pos) = offset;
        *reinterpret_cast<offset_t*>(get_data() + pos + OFFSET_T_SIZE) = tuple_size;
    }

    /**
     * slot is empty when the tuple record's offset is 0
     */
    inline void delete_tuple_record(offset_t slot_num) {
        size_t_ tuple_count = get_tuple_count();
        if (slot_num >= tuple_count)
            return;

        offset_t tuple_record_offset = FIRST_TUPLE_OFFSET + slot_num * TUPLE_RECORD_SZ;
        memset(get_data() + tuple_record_offset, 0, TUPLE_RECORD_SZ);
    }

    offset_t get_empty_slot();
};
    
} // namespace dawn