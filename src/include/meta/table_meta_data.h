#pragma once

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/schema.h"
#include "buffer/buffer_pool_manager.h"
#include "data/types.h"
#include "table/table.h"

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
    /**
     * TODO read the key index
     * create table with meta table, in other words, this table exists in the disk 
     */
    explicit TableMetaData(BufferPoolManager *bpm, const string_t &table_name, const table_id_t table_id);

    /**
     * TODO key index
     * create table from scratch and write data to disk for persistence
     */
    TableMetaData(BufferPoolManager *bpm, const string_t &table_name, const Schema &schema, const table_id_t table_id);

    ~TableMetaData() {
        delete table_;
        delete table_schema_;
        bpm_->unpin_page(self_page_id_, true); // always true
    }

    inline const Schema* get_table_schema() const { return table_schema_; }
    inline string_t get_table_name() const { return table_name_; }
    inline Table* get_table() const { return table_; }
    inline table_id_t get_self_table_id() const { return self_page_id_; }
    inline table_id_t get_table_id() const { return table_id_; }
    inline void set_table_name(const std::string& new_name){ table_name_ = new_name;};
    
    void delete_table_data();
private:
    static const offset_t FIRST_TABLE_PGID_OFFSET = COM_PG_HEADER_SZ;
    static const offset_t INDEX_HEADER_PGID_OFFSET = COM_PG_HEADER_SZ + sizeof(page_id_t);
    static const offset_t COLUMN_NUM_OFFSET = COM_PG_HEADER_SZ + 2*sizeof(page_id_t) + 64; // 64 is reserved space
    static const offset_t FIRST_COLUMN_OFFSET = COM_PG_HEADER_SZ + 2*sizeof(page_id_t) + 64 + sizeof(size_t_);

    /**
     * column size means how many space a column need to record his info.
     * fixed size refer to the space it always needs.
     */
    static const size_t_ FIXED_COLUMN_SIZE = SIZE_T_SIZE + OFFSET_T_SIZE + ENUM_SIZE + SIZE_T_SIZE;
    
    Schema *table_schema_;
    ReaderWriterLatch latch_;
    BufferPoolManager *bpm_;
    string_t table_name_;
    Table *table_;

    // this two ids can't be modified, even the table has no tuple
    page_id_t first_table_page_id_;
    page_id_t index_header_page_id_;
    page_id_t self_page_id_; // where stores the table_meta_data's info
    table_id_t table_id_; // it's also the page id
    Page *page_;
    char *data_;
};

} // namespace dawn
