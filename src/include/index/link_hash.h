#pragma once

#include "util/config.h"
#include "util/util.h"
#include "table/rid.h"
#include "table/tuple.h"

/**
 * link hash needs pages divided into three levels.
 * 
 * first level: store the page ids of the second level's page
 * second level: divided into slots which contains the page id of the page storing data
 * third level: this is a double linked list and it's head page is refered by the second level page's slot
 */

namespace dawn {

op_code_t lk_ha_insert_tuple(page_id_t first_page_id, const Tuple *tuple, const TableSchema &tb_schema);

op_code_t lk_ha_mark_delete(page_id_t first_page_id, const RID &rid);

void lk_ha_apply_delete(page_id_t first_page_id, const RID &rid);

void lk_ha_rollback_delete(page_id_t first_page_id, const RID &rid);

op_code_t lk_ha_get_tuple(page_id_t first_page_id, Tuple *tuple, const TableSchema &tb_schema);

op_code_t lk_ha_update_tuple(page_id_t first_page_id, const Tuple &tuple, const RID &rid, const TableSchema &tb_schema);

} // namespace dawn
