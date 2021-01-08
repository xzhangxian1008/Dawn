#include "meta/table_meta_data.h"

namespace dawn {
    TableMetaData::TableMetaData(BufferPoolManager *bpm, const string_t &table_name,
        table_id_t table_id, const page_id_t self_page_id)
        : bpm_(bpm), table_name_(table_name), table_id_(table_id), self_page_id_(self_page_id) {
        // TODO init
    }

} // namespace dawn
