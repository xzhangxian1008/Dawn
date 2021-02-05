#include "index/link_hash.h"

namespace dawn {

bool lk_ha_insert_tuple(page_id_t first_page_id, const Tuple &tuple, RID *rid, const TableSchema &tb_schema) {
    return true;
}

bool lk_ha_mark_delete(page_id_t first_page_id, const RID &rid);

void lk_ha_apply_delete(page_id_t first_page_id, const RID &rid);

void lk_ha_rollback_delete(page_id_t first_page_id, const RID &rid);

bool lk_ha_get_tuple(page_id_t first_page_id, Tuple *tuple, const RID &rid);

bool lk_ha_update_tuple(page_id_t first_page_id, const Tuple &tuple, const RID &rid);
    
} // namespace dawn
