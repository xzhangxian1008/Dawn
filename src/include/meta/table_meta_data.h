#pragma once

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/table_schema.h"
#include "buffer/buffer_pool_manager.h"
#include "table/table.h"

namespace dawn {

class TableMetaData {
public:
    TableMetaData(const page_id_t index_header_page_id, const TableSchema &table_schema,
        BufferPoolManager *bpm, const page_id_t first_table_page_id, const string_t &table_name,
        table_id_t table_id, Table *table)
        : index_header_page_id_(index_header_page_id), table_schema_(table_schema),
        bpm_(bpm), first_table_page_id_(first_table_page_id), table_name_(table_name),
        table_id_(table_id), table_(table) {}

    ~TableMetaData() {
        // TODO other things should be done
        delete table_;
    }

    

private:
    page_id_t index_header_page_id_;
    TableSchema table_schema_;
    ReaderWriterLatch latch_;
    BufferPoolManager *bpm_;
    string_t table_name_;
    table_id_t table_id_;
    Table *table_;

    // this can't be modified, even the table has no tuple
    const page_id_t first_table_page_id_;
};

} // namespace dawn
