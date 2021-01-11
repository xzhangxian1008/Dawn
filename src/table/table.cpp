#include "table/table.h"

namespace dawn {

Table::Table(BufferPoolManager *bpm, const page_id_t first_table_page_id, bool from_scratch = false) :
    bpm_(bpm), first_table_page_id_(first_table_page_id) {
    if (!from_scratch)
        return;
    
    
}

} // namespace dawn
