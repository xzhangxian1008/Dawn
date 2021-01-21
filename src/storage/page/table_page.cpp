#include "storage/page/table_page.h"

namespace dawn {

void TablePage::init(const page_id_t prev_pgid, const page_id_t next_pgid) {
    memset(get_data(), 0, PAGE_SIZE);
    *reinterpret_cast<page_id_t*>(get_data() + PREV_PGID_OFFSET) = prev_pgid;
    *reinterpret_cast<page_id_t*>(get_data() + NEXT_PGID_OFFSET) = next_pgid;
    *reinterpret_cast<offset_t*>(get_data() + FREE_SPACE_PTR_OFFSET) = INVALID_FREE_SPACE_PTR;
    *reinterpret_cast<size_t_*>(get_data() + TUPLE_CNT_OFFSET) = 0;
}



} // namespace dawn
