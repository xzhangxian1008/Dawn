#include "index/link_hash.h"
#include "storage/page/link_hash_page.h"
#include "storage/page/table_page.h"

namespace dawn {

/**
 * the return value will be set in the second and third parameter
 * @param hash_val the input hash value
 * @param sec_pg_slot_num get the second level's page id with this
 * @param tb_pg_slot_num get the third level's page id with this
 */
inline void get_inserted_slot(hash_t hash_val, offset_t *sec_pg_slot_num, offset_t *tb_pg_slot_num) {
    offset_t slot_num = static_cast<offset_t>(hash_val % static_cast<hash_t>(Lk_HA_TOTAL_SLOT_NUM));
    *sec_pg_slot_num = slot_num / LK_HA_PG_SLOT_NUM;
    *tb_pg_slot_num = slot_num % LK_HA_PG_SLOT_NUM;
}

/**
 * check if the key has existed
 * @param dup_rid duplicate key's position will be set in the dup_rid if the dup_rid is not equal to nullptr
 * @return true: duplicate, false: not duplicate
 */
bool lk_ha_check_duplicate_key(page_id_t first_page_id, const Value &key_value, const TableSchema &tb_schema, BufferPoolManager *bpm, RID *dup_rid = nullptr) {
    // hash the value
    hash_t hash_val = key_value.get_hash_value();
    offset_t sec_pg_slot_num;
    offset_t tb_pg_slot_num;

    get_inserted_slot(hash_val, &sec_pg_slot_num, &tb_pg_slot_num);

    // the the first level's LinkHashPage
    LinkHashPage *first_level_page = reinterpret_cast<LinkHashPage*>(bpm->get_page(first_page_id));
    
    // get the second level's LinkHashPage
    LinkHashPage *second_level_page;
    first_level_page->r_lock();

    page_id_t second_level_page_id = first_level_page->get_pgid_in_slot(sec_pg_slot_num);
    first_level_page->r_unlock();
    bpm->unpin_page(first_page_id, false);

    if (second_level_page_id == INVALID_PAGE_ID) {
        return false;
    }

    second_level_page = reinterpret_cast<LinkHashPage*>(bpm->get_page(second_level_page_id));

    // get the third level's TablePage
    TablePage *third_level_page;
    second_level_page->r_lock();
    page_id_t third_level_page_id = second_level_page->get_pgid_in_slot(tb_pg_slot_num);
    second_level_page->r_unlock();
    bpm->unpin_page(second_level_page_id, false);

    while (third_level_page_id != INVALID_PAGE_ID) {
        Tuple cmp_tuple;
        RID rid;
        third_level_page = reinterpret_cast<TablePage*>(bpm->get_page(third_level_page_id));
        third_level_page->r_lock();
        if (!third_level_page->get_the_first_tuple(&cmp_tuple)) {
            third_level_page->r_unlock();
            bpm->unpin_page(third_level_page_id, false);
            return false;
        }

        // check duplicate in this TablePage
        rid = cmp_tuple.get_rid();
        do {
            third_level_page->get_tuple(&cmp_tuple, rid);
            if (key_value == cmp_tuple.get_value(tb_schema, tb_schema.get_key_idx())) {
                third_level_page->r_unlock();
                bpm->unpin_page(third_level_page_id, false);
                if (dup_rid != nullptr) {
                    *dup_rid = rid;
                }
                return true; // find duplicate key
            }
        } while (third_level_page->get_next_tuple_rid(cmp_tuple.get_rid(), &rid));

        // jump to the next page
        third_level_page_id = third_level_page->get_next_page_id();
        third_level_page->r_unlock();
        bpm->unpin_page(third_level_page->get_page_id(), false);
    }
    return false;
}

op_code_t lk_ha_insert_tuple(INSERT_TUPLE_FUNC_PARAMS) {
    if (lk_ha_check_duplicate_key(first_page_id, (*tuple).get_value(tb_schema, tb_schema.get_key_idx()), tb_schema, bpm)) {
        return DUP_KEY;
    }

    offset_t key_idx = tb_schema.get_key_idx();
    Value key_value = tuple->get_value(tb_schema, key_idx);

    // hash the value
    hash_t hash_val = key_value.get_hash_value();
    offset_t sec_pg_slot_num;
    offset_t tb_pg_slot_num;

    get_inserted_slot(hash_val, &sec_pg_slot_num, &tb_pg_slot_num);
    
    // the the first level's LinkHashPage
    LinkHashPage *first_level_page = reinterpret_cast<LinkHashPage*>(bpm->get_page(first_page_id));
    
    // get the second level's LinkHashPage
    LinkHashPage *second_level_page;
    first_level_page->r_lock();
    page_id_t second_level_page_id = first_level_page->get_pgid_in_slot(sec_pg_slot_num);
    first_level_page->r_unlock();
    bool first_level_pg_dirty = false;

    if (second_level_page_id == INVALID_PAGE_ID) {
        // create the second level page when it's nonexistent
        second_level_page = reinterpret_cast<LinkHashPage*>(bpm->new_page());
        second_level_page_id = second_level_page->get_page_id();
        second_level_page->init();

        // update the first level page
        first_level_page->w_lock();
        first_level_page->set_pgid_in_slot(sec_pg_slot_num, second_level_page_id);
        first_level_page->w_unlock();
        first_level_pg_dirty = true;
    } else {
        second_level_page = reinterpret_cast<LinkHashPage*>(bpm->get_page(second_level_page_id));
    }

    bpm->unpin_page(first_page_id, first_level_pg_dirty);

    // get the third level's TablePage
    TablePage *third_level_page;
    second_level_page->r_lock();
    page_id_t third_level_page_id = second_level_page->get_pgid_in_slot(tb_pg_slot_num);
    second_level_page->r_unlock();
    bool second_level_pg_dirty = false;

    if (third_level_page_id == INVALID_PAGE_ID) {
        // create the third level page when it's nonexistent
        third_level_page = reinterpret_cast<TablePage*>(bpm->new_page());
        third_level_page_id = third_level_page->get_page_id();
        third_level_page->init(INVALID_PAGE_ID, INVALID_PAGE_ID);
        second_level_page->w_lock();
        second_level_page->set_pgid_in_slot(tb_pg_slot_num, third_level_page_id);
        second_level_page->w_unlock();
        second_level_pg_dirty = true;
    } else {
        third_level_page = reinterpret_cast<TablePage*>(bpm->get_page(third_level_page_id));
    }

    bpm->unpin_page(second_level_page_id, second_level_pg_dirty);

    // insert the tuple
    RID rid;
    third_level_page->w_lock();
    while (!third_level_page->insert_tuple(*tuple, &rid)) {
        // jump to the next TablePage or create a new TablePage
        third_level_page_id = third_level_page->get_next_page_id();
        if (third_level_page_id == INVALID_PAGE_ID) {
            // create a new TablePage
            TablePage *new_page = reinterpret_cast<TablePage*>(bpm->new_page());
            if (new_page == nullptr) {
                bpm->unpin_page(third_level_page->get_page_id(), false);
                return NEW_PG_FAIL;
            }

            new_page->w_lock();

            // update the link list
            third_level_page->set_next_page_id(new_page->get_page_id());
            new_page->set_prev_page_id(third_level_page->get_page_id());

            third_level_page->w_unlock();
            bpm->unpin_page(third_level_page->get_page_id(), true);

            // jump to the new TablePage
            third_level_page = new_page;
            third_level_page_id = third_level_page->get_page_id();
            continue;
        }

        // get the next apge
        TablePage *next_page = reinterpret_cast<TablePage*>(bpm->get_page(third_level_page_id));
        third_level_page->w_unlock();
        bpm->unpin_page(third_level_page->get_page_id(), false);

        // jump to the next page
        third_level_page = next_page;
        third_level_page->w_lock();
    }
    third_level_page->w_unlock();
    bpm->unpin_page(third_level_page_id, true);
    tuple->set_rid(rid);
    return OP_SUCCESS;
}

op_code_t lk_ha_mark_delete(MARK_DELETE_FUNC_PARAMS) {
    return OP_SUCCESS;
}

void lk_ha_apply_delete(APPLY_DELETE_FUNC_PARAMS) {

}

void lk_ha_rollback_delete(ROLLBACK_DELETE_FUNC_PARAMS) {
    
}

/**
 * @param tuple tuple is return by this pointer
 */
op_code_t lk_ha_get_tuple(GET_TUPLE_FUNC_PARAMS) {
    RID rid;
    if (!lk_ha_check_duplicate_key(first_page_id, key_value, tb_schema, bpm, &rid)) {
        // can't find the tuple
        return TUPLE_NOT_FOUND;
    }

    TablePage *table_page = reinterpret_cast<TablePage*>(bpm->get_page(rid.get_page_id()));

    table_page->r_lock();
    if (table_page->get_tuple(tuple, rid)) {
        table_page->r_unlock();
        bpm->unpin_page(rid.get_page_id(), false);
        return OP_SUCCESS;
    }
    table_page->r_unlock();
    bpm->unpin_page(rid.get_page_id(), false);
    LOG("here");
    return TUPLE_NOT_FOUND;
}

op_code_t lk_ha_update_tuple(UPDATE_TUPLE_FUNC_PARAMS) {
    // get the old tuple first
    Value key_val = new_tuple->get_value(tb_schema, tb_schema.get_key_idx());
    Tuple old_tuple;
    if (get_tuple_directly(old_rid, &old_tuple, bpm) != OP_SUCCESS) {
        return TUPLE_NOT_FOUND;
    }

    TablePage *tb_page = reinterpret_cast<TablePage*>(bpm->get_page(old_rid.get_page_id()));

    // compare the key
    if (key_val == old_tuple.get_value(tb_schema, tb_schema.get_key_idx())) {
        // update the tuple in place
        tb_page->w_lock();
        tb_page->update_tuple(*new_tuple, old_rid);
        tb_page->w_unlock();
        bpm->unpin_page(old_rid.get_page_id(), true);
        new_tuple->set_rid(old_rid);
        return OP_SUCCESS;
    }

    op_code_t op_code = lk_ha_insert_tuple(first_page_id, new_tuple, tb_schema, bpm);

    if (op_code == DUP_KEY) {
        return DUP_KEY;
    }

    // delete the key
    tb_page->w_lock();
    if (!tb_page->mark_delete(old_rid)) {
        tb_page->w_unlock();
        bpm->unpin_page(old_rid.get_page_id(), false);
        LOG("should not reach here");
        return MARK_DELETE_FAIL;
    }
    tb_page->apply_delete(old_rid);
    tb_page->w_unlock();
    bpm->unpin_page(old_rid.get_page_id(), true);
    return OP_SUCCESS;
}

} // namespace dawn
