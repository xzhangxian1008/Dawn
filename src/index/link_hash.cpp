#include "index/link_hash.h"
#include "manager/db_manager.h"
#include "storage/page/link_hash_page.h"

namespace dawn {

bool lk_ha_insert_tuple(page_id_t first_page_id, Tuple *tuple, const TableSchema &tb_schema) {
    offset_t key_idx = tb_schema.get_key_idx();
    size_t_ key_size = tb_schema.get_column_size(key_idx);
    Value key_value = tuple->get_value(tb_schema, key_idx);
    
    BufferPoolManager *bpm = db_ptr->get_buffer_pool_manager();
    LinkHashPage *lkh_page = reinterpret_cast<LinkHashPage*>(bpm->get_page(first_page_id));
    
}

bool lk_ha_mark_delete(page_id_t first_page_id, const RID &rid);

void lk_ha_apply_delete(page_id_t first_page_id, const RID &rid);

void lk_ha_rollback_delete(page_id_t first_page_id, const RID &rid);

bool lk_ha_get_tuple(page_id_t first_page_id, Tuple *tuple, const RID &rid);

bool lk_ha_update_tuple(page_id_t first_page_id, const Tuple &tuple, const RID &rid);
    
} // namespace dawn
