#pragma once

#include "util/config.h"
#include "table/tuple.h"
#include "table/tb_common_op.h"

/**
 * link hash needs pages divided into three levels.
 * 
 * first level: store the page ids of the second level's page
 * second level: divided into slots which contains the page id of the page storing data
 * third level: this is a double linked list and it's head page is refered by the second level page's slot
 */

namespace dawn {

/**
 * the link hash insert operation is divided into three level
 * first level: only one LinkHashPage, his slots store the page ids that refer to the second level's LinkHashPage
 * second level: many LinkHashPage, their slots store the page ids that refer to the third level's TablePage
 * third level: the actual level that stores data and has many double linked list.
 * @param first_page_id refer to the first level's page id
 * @param tuple insert it's data into db and set it's RID to return the insert position
 * @param tb_schema describe the tuple to get the key index
 */
op_code_t lk_ha_insert_tuple(INSERT_TUPLE_FUNC_PARAMS);

// may be useless
op_code_t lk_ha_mark_delete(MARK_DELETE_FUNC_PARAMS);

// may be useless
void lk_ha_apply_delete(APPLY_DELETE_FUNC_PARAMS);

// may be useless
void lk_ha_rollback_delete(ROLLBACK_DELETE_FUNC_PARAMS);

op_code_t lk_ha_get_tuple(GET_TUPLE_FUNC_PARAMS);

/**
 * Firstly, check if key will be modified. 
 * Yes, then reinsert the new_tuple, but may be fail because of the possible duplicate, and delete the old tuple.
 * No, modify the tuple in place.
 * @param new_tuple new position will be set in the new tuple
 * @param old_rid old tuple's position
 */
op_code_t lk_ha_update_tuple(UPDATE_TUPLE_FUNC_PARAMS);

} // namespace dawn
