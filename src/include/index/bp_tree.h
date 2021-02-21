#pragma once

#include "buffer/buffer_pool_manager.h"
#include "util/config.h"
#include "util/util.h"
#include "util/hash_function.h"
#include "table/rid.h"
#include "table/tuple.h"

namespace dawn {

// bool bpt_insert_tuple(page_id_t first_page_id, const Tuple &tuple, RID *rid);

// bool bpt_mark_delete(page_id_t first_page_id, const RID &rid);

// void bpt_apply_delete(page_id_t first_page_id, const RID &rid);

// void bpt_rollback_delete(page_id_t first_page_id, const RID &rid);

// bool bpt_get_tuple(page_id_t first_page_id, Tuple *tuple, const RID &rid);

// bool bpt_update_tuple(page_id_t first_page_id, const Tuple &tuple, const RID &rid);

} // namespace dawn