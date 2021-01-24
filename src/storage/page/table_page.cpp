#include "storage/page/table_page.h"

namespace dawn {

void TablePage::init(const page_id_t prev_pgid, const page_id_t next_pgid) {
    memset(get_data(), 0, PAGE_SIZE);
    *reinterpret_cast<page_id_t*>(get_data() + PREV_PGID_OFFSET) = prev_pgid;
    *reinterpret_cast<page_id_t*>(get_data() + NEXT_PGID_OFFSET) = next_pgid;
    *reinterpret_cast<size_t_*>(get_data() + TUPLE_CNT_OFFSET) = 0;
    set_free_space_pointer(INVALID_FREE_SPACE_PTR);
}

bool TablePage::insert_tuple(const Tuple &tuple, RID *rid) {
    // check we have enough space to insert the tuple
    size_t_ free_space = get_free_space();
    size_t_ tuple_size = tuple.get_size();
    if (free_space < tuple_size + TUPLE_RECORD_SZ) {
        return false;
    }
    
    // write data
    offset_t free_space_pointer = get_free_space_pointer();
    free_space_pointer -= tuple_size;

    tuple.serialize_to(static_cast<char*>(get_data() + free_space_pointer));
    set_free_space_pointer(free_space_pointer);

    // insert tuple's record
    size_t_ tuple_count = get_tuple_count();
    offset_t insert_record_offset = FIRST_TUPLE_OFFSET + TUPLE_RECORD_SZ * tuple_count;

    insert_tuple_record(insert_record_offset, free_space_pointer, tuple_size);
}

bool TablePage::mark_delete(const RID &rid) {
    offset_t slot_num = rid.get_slot_num();
    if (slot_num >= get_tuple_count())
        return false;

    size_t_ tuple_size = get_tuple_size(slot_num);
    set_tuple_size(slot_num, set_deleted_flag(tuple_size));
    return true;
}

void TablePage::rollback_delete(const RID &rid) {
    offset_t slot_num = rid.get_slot_num();
    if (slot_num >= get_tuple_count())
        return;
    
    size_t_ tuple_size = get_tuple_size(slot_num);
    set_tuple_size(slot_num, unset_deleted_flag(tuple_size));
}

void TablePage::apply_delete(const RID &rid) {
    offset_t slot_num = rid.get_slot_num();
    if (slot_num >= get_tuple_count())
        return;
    
    size_t_ tuple_size = unset_deleted_flag(get_tuple_size(slot_num));
    offset_t tuple_offset = 
}

} // namespace dawn
