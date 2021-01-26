#include "storage/page/table_page.h"

namespace dawn {

void TablePage::init(const page_id_t prev_pgid, const page_id_t next_pgid) {
    memset(get_data(), 0, PAGE_SIZE);
    *reinterpret_cast<page_id_t*>(get_data() + PREV_PGID_OFFSET) = prev_pgid;
    *reinterpret_cast<page_id_t*>(get_data() + NEXT_PGID_OFFSET) = next_pgid;
    *reinterpret_cast<size_t_*>(get_data() + TUPLE_CNT_OFFSET) = 0;
    set_free_space_pointer(INVALID_FREE_SPACE_PTR);
}

// TODO slow, may be we can use bitmap?
offset_t TablePage::get_empty_slot() {
    size_t_ tuple_count = get_tuple_count();
    for (size_t_ slot_num = 0; slot_num < tuple_count; slot_num++) {
        offset_t offset = get_tuple_offset(slot_num);
        if (offset == 0)
            return slot_num;
    }

    size_t_ free_space = get_free_space();
    if (free_space < TUPLE_RECORD_SZ)
        return -1;

    set_tuple_count(tuple_count + 1);
    return tuple_count;
}

bool TablePage::insert_tuple(const Tuple &tuple, RID *rid) {
    // check we have enough space to insert the tuple
    offset_t inserted_slot_num = get_empty_slot();
    size_t_ free_space = get_free_space();
    size_t_ tuple_size = tuple.get_size();
    if (free_space < tuple_size) {
        return false;
    }
    
    // write data
    offset_t free_space_pointer = get_free_space_pointer();

    free_space_pointer -= tuple_size;
    tuple.serialize_to(static_cast<char*>(get_data() + free_space_pointer));
    set_free_space_pointer(free_space_pointer);

    // update RID
    rid->set(get_page_id(), inserted_slot_num);

    // insert tuple's record
    insert_tuple_record(inserted_slot_num, free_space_pointer, tuple_size);
    return true;
}

bool TablePage::mark_delete(const RID &rid) {
    offset_t slot_num = rid.get_slot_num();
    offset_t offset = get_tuple_offset(slot_num);
    page_id_t page_id = get_page_id();
    if (slot_num >= get_tuple_count() || offset == 0 || page_id != rid.get_page_id())
        return false;

    size_t_ tuple_size = get_tuple_size(slot_num);
    set_tuple_size(slot_num, set_deleted_flag(tuple_size));
    return true;
}

void TablePage::rollback_delete(const RID &rid) {
    offset_t slot_num = rid.get_slot_num();
    offset_t offset = get_tuple_offset(slot_num);
    if (slot_num >= get_tuple_count() || offset == 0)
        return;
    
    size_t_ tuple_size = get_tuple_size(slot_num);
    set_tuple_size(slot_num, unset_deleted_flag(tuple_size));
}

void TablePage::apply_delete(const RID &rid) {
    offset_t deleted_slot_num = rid.get_slot_num();
    offset_t deleted_tuple_offset = get_tuple_offset(deleted_slot_num);
    size_t_ deleted_tuple_size = get_tuple_size(deleted_slot_num);
    if ((deleted_slot_num >= get_tuple_count() || deleted_tuple_offset == 0) && is_deleted(deleted_tuple_size))
        return;
    
    offset_t free_space_pointer = get_free_space_pointer();
    deleted_tuple_size = unset_deleted_flag(get_tuple_size(deleted_slot_num));
    
    // delete the tuple's record
    delete_tuple_record(deleted_slot_num);

    // move the tuple's data
    memmove(get_data() + free_space_pointer + deleted_tuple_size, 
        get_data() + free_space_pointer, 
        deleted_tuple_offset - free_space_pointer);

    // update free space pointer
    set_free_space_pointer(free_space_pointer + deleted_tuple_size);

    // update tuples' record
    size_t_ tuple_count = get_tuple_count();
    for (size_t_ slot_num = 0; slot_num < tuple_count; slot_num++) {
        offset_t tuple_offset = get_tuple_offset(slot_num);
        if (tuple_offset != 0 && tuple_offset < deleted_tuple_offset)
            set_tuple_offset(slot_num, tuple_offset + deleted_tuple_size);
    }
}

bool TablePage::update_tuple(const Tuple &new_tuple, const RID &rid) {
    offset_t slot_num = rid.get_slot_num();
    offset_t tuple_offset = get_tuple_offset(slot_num);
    size_t_ tuple_size = get_tuple_size(slot_num);
    page_id_t page_id = rid.get_page_id();

    // check if it's legal
    if (slot_num >= get_tuple_count() || tuple_offset == 0 || 
        tuple_size != new_tuple.get_size() || page_id != get_page_id())
        return false;
    
    new_tuple.serialize_to(get_data() + tuple_offset);

    return true;
}

bool TablePage::get_tuple(Tuple *tuple, const RID &rid) const {
    offset_t slot_num = rid.get_slot_num();
    offset_t tuple_offset = get_tuple_offset(slot_num);
    size_t_ tuple_size = get_tuple_size(slot_num);

    // check if it's legal
    if (slot_num >= get_tuple_count() || tuple_offset == 0)
        return false;
    
    tuple->size_ = tuple_size;
    if (tuple->allocated_) {
        delete[] tuple->data_;
    }
    tuple->data_ = new char[tuple_size];
    tuple->allocated_ = true;
    tuple->set_rid(rid);
    tuple->deserialize_from(get_data() + tuple_offset);

    return true;
}

bool TablePage::get_next_tuple_rid(const RID &cur_rid, RID *next_rid) const {
    offset_t slot_num = cur_rid.get_slot_num();
    size_t_ tuple_count = get_tuple_count();
    for (size_t_ i = slot_num; i < tuple_count; i++) {
        offset_t offset = get_tuple_offset(i);
        if (offset == 0)
            continue;
        next_rid->set(get_page_id(), i);
        return true;
    }

    next_rid->set(INVALID_PAGE_ID, -1);
    return false;
}

} // namespace dawn
