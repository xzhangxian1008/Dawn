#include "index/link_hash.h"
#include "manager/db_manager.h"
#include "storage/page/link_hash_page.h"

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
 * @return true: duplicate, false: not duplicate
 */
inline bool check_tuple_dup_key(const Tuple &t1, const Tuple &t2, const TableSchema &tb_schema) {
    offset_t key_idx = tb_schema.get_key_idx();
    Value v1 = t1.get_value(tb_schema, key_idx);
    Value v2 = t2.get_value(tb_schema, key_idx);

    if (v1 == v2) {
        return true;
    }
    return false;
}

/**
 * check if the key has existed
 * @return true: duplicate, false: not duplicate
 */
bool check_duplicate_key(page_id_t first_page_id, const Tuple &tuple, const TableSchema &tb_schema) {
    offset_t key_idx = tb_schema.get_key_idx();
    size_t_ key_size = tb_schema.get_column_size(key_idx);
    Value key_value = tuple.get_value(tb_schema, key_idx);

    // hash the value
    hash_t hash_val = key_value.get_hash_value();
    offset_t sec_pg_slot_num;
    offset_t tb_pg_slot_num;

    get_inserted_slot(hash_val, &sec_pg_slot_num, &tb_pg_slot_num);

    // the the first level's LinkHashPage
    BufferPoolManager *bpm = db_ptr->get_buffer_pool_manager();
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
            if (check_tuple_dup_key(tuple, cmp_tuple, tb_schema)) {
                third_level_page->r_unlock();
                bpm->unpin_page(third_level_page_id, false);
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
 * the link hash insert operation is divided into three level
 * first level: only one LinkHashPage, his slots store the page ids that refer to the second level's LinkHashPage
 * second level: many LinkHashPage, their slots store the page ids that refer to the third level's TablePage
 * third level: the actual level that stores data and has many double linked list.
 * @param first_page_id refer to the first level's page id
 * @param tuple insert it's data into db and set it's RID to return the insert position
 * @param tb_schema describe the tuple to get the key index
 */
op_code_t lk_ha_insert_tuple(page_id_t first_page_id, Tuple *tuple, const TableSchema &tb_schema) {
    if (check_duplicate_key(first_page_id, *tuple, tb_schema)) {
        return DUP_KEY;
    }

    offset_t key_idx = tb_schema.get_key_idx();
    size_t_ key_size = tb_schema.get_column_size(key_idx);
    Value key_value = tuple->get_value(tb_schema, key_idx);

    // hash the value
    hash_t hash_val = key_value.get_hash_value();
    offset_t sec_pg_slot_num;
    offset_t tb_pg_slot_num;

    get_inserted_slot(hash_val, &sec_pg_slot_num, &tb_pg_slot_num);
    
    // the the first level's LinkHashPage
    BufferPoolManager *bpm = db_ptr->get_buffer_pool_manager();
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

        // update the second level page
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
    return true;
}

op_code_t lk_ha_mark_delete(page_id_t first_page_id, const RID &rid);

void lk_ha_apply_delete(page_id_t first_page_id, const RID &rid);

void lk_ha_rollback_delete(page_id_t first_page_id, const RID &rid);

op_code_t lk_ha_get_tuple(page_id_t first_page_id, Tuple *tuple, const RID &rid);

op_code_t lk_ha_update_tuple(page_id_t first_page_id, const Tuple &tuple, const RID &rid) {
    
}

} // namespace dawn
