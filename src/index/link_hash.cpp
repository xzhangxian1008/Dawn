#include "index/link_hash.h"
#include "storage/page/link_hash_page.h"
#include "storage/page/table_page.h"

namespace dawn {

/**
 * the returned value will be set in the second and third parameter
 * @param hash_val the input hash value
 * @param sec_pg_slot_num get the second level's page id with this
 * @param tb_pg_slot_num get the third level's page id with this
 */
inline void hash_to_slot(hash_t hash_val, offset_t *sec_pg_slot_num, offset_t *tb_pg_slot_num) {
    offset_t slot_num = static_cast<offset_t>(hash_val % static_cast<hash_t>(Lk_HA_TOTAL_SLOT_NUM));
    *sec_pg_slot_num = slot_num / LK_HA_PG_SLOT_NUM;
    *tb_pg_slot_num = slot_num % LK_HA_PG_SLOT_NUM;
}

/**
 * check if the key has existed
 * @param dup_rid duplicate key's position will be set in the dup_rid if the dup_rid is not equal to nullptr
 * @return true: duplicate, false: not duplicate
 */
bool lk_ha_check_duplicate_key(page_id_t first_page_id, const Value &key_value, const Schema &tb_schema, BufferPoolManager *bpm, RID *dup_rid = nullptr) {
    // hash the value
    hash_t hash_val = key_value.get_hash_value();
    offset_t sec_pg_slot_num;
    offset_t tb_pg_slot_num;

    hash_to_slot(hash_val, &sec_pg_slot_num, &tb_pg_slot_num);

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

/**
 * It scans the TablePage list and deletes the empty page.
 * It's called every time when we delete the last tuple in a TablePage,
 * but in the concurrent environment the empty page may be filled with
 * tuples before deleted, so the lk_ha_clear_empty_page() is a function
 * that checks and deletes the empty pages instead of ensuring to delete a page.
 * 
 * So far, we can only delete the empty pages at the beginning. Empty pages in the middle
 * of the link list is ignored, because we can't test this function and there is no need to implement it.
 * 
 * For convenience, the last several unpin function's dirty flag is alway true.
 * We can control it when coming across any performance problem in this function.
 * 
 * Hold the locks of second level page and empty pages, because the link list may be changed
 * and we should resist others to access this list when it's unstable.
 * 
 * @param sec_pg_slot_num slot_num offset in the first level page, used for getting the second level page id
 * @param tb_pg_slot_num slot_num offset int the second level page, used for getting the first TablePage's page id
 */
void lk_ha_clear_empty_page(page_id_t first_page_id, BufferPoolManager *bpm, hash_t hash_val, offset_t sec_pg_slot_num, offset_t tb_pg_slot_num) {
    // get the first level's page
    LinkHashPage *first_level_pg = reinterpret_cast<LinkHashPage*>(bpm->get_page(first_page_id));

    // get the second level's page
    first_level_pg->r_lock();
    page_id_t sec_level_pgid = first_level_pg->get_pgid_in_slot(sec_pg_slot_num);
    first_level_pg->r_unlock();
    bpm->unpin_page(first_page_id, false);

    LinkHashPage *sec_level_pg = reinterpret_cast<LinkHashPage*>(bpm->get_page(sec_level_pgid));

    // get the third level's first TablePage
    sec_level_pg->w_lock(); // the second level page may be changed, so hold the write lock all the time
    page_id_t checked_pgid = sec_level_pg->get_pgid_in_slot(tb_pg_slot_num);

    /**
     * ATTENTION Do not release the lock when collecting the empty pages
     * 
     * there may be several empty TablePage that are linked together at the beginning.
     * Collect them and find the first non-empty TablePage.
     * Delete the collected empty TablePages and change the page id in the second level
     * page's slot to refer to the first non-empty TablePage.
     */
    std::vector<TablePage*> empty_pages; // store the empty pages that should be deleted
    TablePage *checked_tb_page = reinterpret_cast<TablePage*>(bpm->get_page(checked_pgid));
    checked_tb_page->w_lock();
    while (checked_tb_page->get_stored_tuple_cnt() == 0) {
        empty_pages.push_back(checked_tb_page);

        // get the next page
        checked_pgid = checked_tb_page->get_next_page_id();

        // reach to the end, break out
        if (checked_pgid == INVALID_PAGE_ID)
            break;
        
        // jump to the next page
        checked_tb_page->w_lock();
        checked_tb_page = reinterpret_cast<TablePage*>(bpm->get_page(checked_pgid));
    }

    // delete the empty pages
    if (empty_pages.size() > 0) {
        for (size_t i = 0; i < empty_pages.size(); i++) {
            empty_pages[i]->w_unlock();
            bpm->unpin_page(empty_pages[i]->get_page_id(), false);
            while (true) {
                if (bpm->delete_page(empty_pages[i]->get_page_id())) {
                    break;
                }

                // someone else is accessing the page, wait...
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    }

    // reset the page id in the second level page's slot
    sec_level_pg->set_pgid_in_slot(tb_pg_slot_num, checked_pgid);

    // there is no non-empty pages in this slot
    if (checked_pgid == INVALID_PAGE_ID) {
        sec_level_pg->w_unlock();
        bpm->unpin_page(sec_level_pgid, true); // for convenience, always true
        return;
    }

    // set the first non-empty page's previous page id to the INVALID_PAGE_ID
    checked_tb_page->set_prev_page_id(INVALID_PAGE_ID);
    checked_tb_page->w_unlock();
    sec_level_pg->w_unlock();
    bpm->unpin_page(checked_pgid, true); // for convenience, always true
    bpm->unpin_page(sec_level_pgid, true); // for convenience, always true
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

    hash_to_slot(hash_val, &sec_pg_slot_num, &tb_pg_slot_num);
    
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

op_code_t lk_ha_mark_delete(MARK_DELETE_FUNC_PARAMS) {
    Tuple tuple;
    if (!lk_ha_get_tuple(first_page_id, key_value, &tuple, tb_schema, bpm)) {
        return false;
    }

    page_id_t page_id = tuple.get_rid().get_page_id();
    TablePage *table_page = reinterpret_cast<TablePage*>(bpm->get_page(page_id));

    table_page->w_lock();
    bool ok = table_page->mark_delete(tuple.get_rid());
    table_page->w_unlock();
    bpm->unpin_page(page_id, true);

    if (ok)
        return OP_SUCCESS;
    else
        return TUPLE_NOT_FOUND;
}

void lk_ha_apply_delete(APPLY_DELETE_FUNC_PARAMS) {
    Tuple tuple;
    if (lk_ha_get_tuple(first_page_id, key_value, &tuple, tb_schema, bpm) != OP_SUCCESS) {
        return;
    }

    page_id_t page_id = tuple.get_rid().get_page_id();
    TablePage *tb_page = reinterpret_cast<TablePage*>(bpm->get_page(page_id));
    tb_page->w_lock();
    tb_page->apply_delete(tuple.get_rid());
    size_t_ tuple_cnt = tb_page->get_stored_tuple_cnt();
    tb_page->w_unlock();
    bpm->unpin_page(page_id, true);
    if (tuple_cnt == 0) {
        offset_t sec_pg_slot_num;
        offset_t tb_pg_slot_num;
        hash_to_slot(key_value.get_hash_value(), &sec_pg_slot_num, &tb_pg_slot_num);
        lk_ha_clear_empty_page(first_page_id, bpm, key_value.get_hash_value(), sec_pg_slot_num, tb_pg_slot_num);
    }
}

// TODO need implementation
void lk_ha_rollback_delete(ROLLBACK_DELETE_FUNC_PARAMS) {

}

} // namespace dawn
