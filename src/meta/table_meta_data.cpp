#include "meta/table_meta_data.h"

namespace dawn {
/** 
 * create TableMetaData with previous stored data
 */
TableMetaData::TableMetaData(BufferPoolManager *bpm, const string_t &table_name, const table_id_t table_id)
    : bpm_(bpm), table_name_(table_name), table_id_(table_id), self_page_id_((page_id_t)table_id),
    first_table_page_id_(-1), index_header_page_id_(-1) {
    // get data from disk
    page_ = bpm->get_page(self_page_id_);
    if (page_ == nullptr) {
        PRINT("ERROR!: TableMetaData can't be read");
        exit(0);
    }
    data_ = page_->get_data();

    // initialize data
    page_id_t *pgid = const_cast<page_id_t*>(&first_table_page_id_);
    *pgid = *reinterpret_cast<page_id_t*>(data_ + FIRST_TABLE_PGID_OFFSET);
    pgid = const_cast<page_id_t*>(&index_header_page_id_);
    *pgid = *reinterpret_cast<page_id_t*>(data_ + INDEX_HEADER_PGID_OFFSET);
    size_t_ column_num = *reinterpret_cast<page_id_t*>(data_ + COLUMN_NUM_OFFSET);

    // construct columns to make TableSchema
    std::vector<Column> cols;
    size_t_ col_name_len;
    offset_t offset_in_tp; // the column data's offset in tuple
    TypeId type_id;
    size_t_ data_size;

    size_t_ col_size = 0; // record how large space this column's info occupy
    size_t_ col_name_len_offset = FIRST_COLUMN_OFFSET;
    size_t_ name_offset;
    size_t_ offset_offset; // interesting name
    size_t_ type_id_offset;
    size_t_ data_size_offset;
    for (size_t_ i = 0; i < column_num; i++) {
        col_name_len_offset += col_size; // jump to the next column

        col_name_len = *reinterpret_cast<size_t_*>(data_ + col_name_len_offset);
        col_size = FIXED_COLUMN_SIZE + col_name_len + 1;
        name_offset = col_name_len_offset + SIZE_T_SIZE;
        offset_offset = name_offset + col_name_len + 1;
        offset_in_tp = *reinterpret_cast<offset_t*>(data_ + offset_offset);
        type_id_offset = offset_offset + OFFSET_T_SIZE;
        type_id = *reinterpret_cast<TypeId*>(data_ + type_id_offset);
        data_size_offset = type_id_offset + ENUM_SIZE;
        data_size = *reinterpret_cast<size_t_*>(data_ + data_size_offset);

        char *name = reinterpret_cast<char*>(data_ + name_offset);

        // make column
        if (type_id == TypeId::CHAR) {
            cols.push_back(Column(name, offset_in_tp, data_size));
        } else {
            cols.push_back(Column(type_id, name, offset_in_tp));
        }
    }

    // create TableSchema
    table_schema_ = new TableSchema(cols);

    // create Table
    table_ = new Table(bpm_, first_table_page_id_, false);
}

/** 
 * create TableMetaData from scratch
 */
TableMetaData::TableMetaData(BufferPoolManager *bpm, const string_t &table_name, const TableSchema &schema, const table_id_t table_id)
    : bpm_(bpm), table_name_(table_name), table_id_(table_id), self_page_id_(table_id), first_table_page_id_(-1), index_header_page_id_(-1) {
    table_schema_ = new TableSchema(schema);

    // firstly, get page from disk
    page_ = bpm->get_page(self_page_id_);
    if (page_ == nullptr) {
        PRINT("ERROR!: TableMetaData can't be read");
        exit(0);
    }
    data_ = page_->get_data();

    size_t_ col_num = schema.get_column_num();

    // write data to the memory
    *reinterpret_cast<size_t_*>(data_ + COLUMN_NUM_OFFSET) = col_num;

    size_t_ col_size = 0; // record how large space this column's info occupy
    size_t_ col_name_len_offset = FIRST_COLUMN_OFFSET;
    size_t_ name_offset;
    size_t_ offset_offset; // interesting name...
    size_t_ type_id_offset;
    size_t_ data_size_offset;
    for (int i = 0; i < col_num; i++) {
        Column col = schema.get_column(i);
        col_name_len_offset += col_size; // jump to the next column

        const string_t &col_name = col.get_column_name();
        size_t_ name_len = col_name.length();
        *reinterpret_cast<size_t_*>(data_ + col_name_len_offset) = name_len;
        name_offset = col_name_len_offset + SIZE_T_SIZE;
        offset_offset = name_offset + name_len + 1;
        *reinterpret_cast<offset_t*>(data_ + offset_offset) = col.get_offset();
        type_id_offset = offset_offset + OFFSET_T_SIZE;
        *reinterpret_cast<TypeId*>(data_ + type_id_offset) = col.get_type_id();
        data_size_offset = type_id_offset + ENUM_SIZE;
        *reinterpret_cast<size_t_*>(data_ + data_size_offset) = col.get_data_size();

        for (offset_t i = 0; i < (offset_t)name_len; i++)
            *reinterpret_cast<char*>(data_ + name_offset + i) = col_name[i];
        *reinterpret_cast<char*>(data_ + name_offset + (offset_t)name_len) = '\0';
        col_size = FIXED_COLUMN_SIZE + name_len + 1;
    }

    // flush TableMetaData's data to disk
    bpm_->flush_page(self_page_id_);

    // create table's first page
    Page *page = bpm_->new_page();
    if (page == nullptr) {
        LOG("ERROR! can't get new page");
        exit(-1);
    }
    page_id_t *pgid = const_cast<page_id_t*>(&first_table_page_id_);
    *pgid = page->get_page_id();
    bpm_->unpin_page(*pgid, false);
    *reinterpret_cast<page_id_t*>(data_ + FIRST_TABLE_PGID_OFFSET) = first_table_page_id_;

    // create index_header's page
    page = bpm_->new_page();
    if (page == nullptr) {
        LOG("ERROR! can't get new page");
        exit(-1);
    }
    pgid = const_cast<page_id_t*>(&index_header_page_id_);
    *pgid = page->get_page_id();
    bpm_->unpin_page(*pgid, false);
    *reinterpret_cast<page_id_t*>(data_ + INDEX_HEADER_PGID_OFFSET) = index_header_page_id_;

    table_ = new Table(bpm_, first_table_page_id_, true);
}

// TODO delete index data(index_header_page_id_)
void TableMetaData::delete_table_data() {
    latch_.w_lock();
    table_->delete_all_data();
    latch_.w_unlock();
}

} // namespace dawn
