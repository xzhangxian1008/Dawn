#pragma once

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/table_schema.h"
#include "table/table.h"
#include "buffer/buffer_pool_manager.h"
#include "table/table.h"
#include "data/types.h"

namespace dawn {

/**
 * ATTENTION the TableMetaData always hold the Page while it is alive, this may be not efficient
 * 
 * TableMetaData header layout:
 * we suppose that one page could store all the info
 * ------------------------------------------------------------------------
 * |                          common header (64)                          |
 * ------------------------------------------------------------------------
 * | first_table_page_id_ (4) | index_header_page_id_ (4) | Reserved (64) |
 * ------------------------------------------------------------------------
 * | column num (4) | col name length 1 (4) | name 1 (x) | offset 1 (4) |
 * ------------------------------------------------------------------------
 * | type id 1 (4) | data size 1 (4) | ... |
 * ------------------------------------------------------------------------
 */
class TableMetaData {
public:
    // create table with meta table, in other words, this table exists in the disk
    explicit TableMetaData(BufferPoolManager *bpm, const string_t &table_name, const table_id_t table_id);

    // create table from scratch and write data to disk for persistence
    TableMetaData(BufferPoolManager *bpm, const string_t &table_name, const TableSchema &schema, const table_id_t table_id);

    ~TableMetaData() {
        // TODO other things should be done
        delete table_;
        delete table_schema_;
        bpm_->unpin_page(self_page_id_, true); // always true
    }

    // TODO add function to new, update, delete the table
    
    void delete_table_meta_data() {} // TODO need impl
private:
    static const offset_t FIRST_TABLE_PGID_OFFSET = COM_PG_HEADER_SZ;
    static const offset_t INDEX_HEADER_PGID_OFFSET = COM_PG_HEADER_SZ + sizeof(page_id_t);
    static const offset_t COLUMN_NUM_OFFSET = COM_PG_HEADER_SZ + 2*sizeof(page_id_t) + 64; // 64 is reserved space
    static const offset_t FIRST_COLUMN_OFFSET = COM_PG_HEADER_SZ + 2*sizeof(page_id_t) + 64 + sizeof(size_t_);

    /**
     * column size means how many space a column need to record his info.
     * fixed size refer to the space it always need.
     */
    static const size_t_ FIXED_COL_SIZE = 2 * sizeof(size_t_) + sizeof(offset_t) + ENUM_SIZE;
    
    TableSchema *table_schema_;
    ReaderWriterLatch latch_;
    BufferPoolManager *bpm_;
    string_t table_name_;
    Table *table_;

    // this two ids can't be modified, even the table has no tuple
    const page_id_t first_table_page_id_;
    const page_id_t index_header_page_id_;
    const page_id_t self_page_id_; // where stores the table_meta_data's info
    const table_id_t table_id_; // it's also the page id
    Page *page_;
    char *data_;
};

} // namespace dawn
