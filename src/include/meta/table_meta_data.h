#pragma once

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/table_schema.h"
#include "buffer/buffer_pool_manager.h"
#include "table/table.h"

namespace dawn {

/**
 * TableMetaData header layout:
 * ------------------------------------------------------------------------
 * |                          common header (64)                          |
 * ------------------------------------------------------------------------
 * | first_table_page_id_ (4) | index_header_page_id_ (4) |
 * ------------------------------------------------------------------------
 * |                             Schema(TODO)                             |
 * ------------------------------------------------------------------------
 */
class TableMetaData {
public:
    TableMetaData(BufferPoolManager *bpm, const string_t &table_name,
        table_id_t table_id, const page_id_t self_page_id);

    ~TableMetaData() {
        // TODO other things should be done
        delete table_;
        delete table_schema_;
    }    

private:
    // TODO need schema designed first to arrange the page layout
    TableSchema *table_schema_;
    ReaderWriterLatch latch_;
    BufferPoolManager *bpm_;
    string_t table_name_;
    table_id_t table_id_;
    Table *table_;

    // this can't be modified, even the table has no tuple
    page_id_t first_table_page_id_;
    page_id_t index_header_page_id_;
    const page_id_t self_page_id_; // where stores the table_meta_data's info
    Page *page_;
    char *data_;
};

} // namespace dawn
