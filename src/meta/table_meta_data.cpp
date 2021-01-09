#include "meta/table_meta_data.h"

namespace dawn {
    // TODO create Table
    TableMetaData::TableMetaData(BufferPoolManager *bpm, const string_t &table_name,
        table_id_t table_id, const page_id_t self_page_id)
        : bpm_(bpm), table_name_(table_name), table_id_(table_id), self_page_id_(self_page_id) {
        // get data from disk
        page_ = bpm->get_page(self_page_id_);
        if (page_ == nullptr) {
            PRINT("ERROR!: TableMetaData can't be read");
            exit(0);
        }
        data_ = page_->get_data();

        // initialize data
        first_table_page_id_ = *reinterpret_cast<page_id_t*>(data_ + FIRST_TABLE_PGID_OFFSET);
        index_header_page_id_ = *reinterpret_cast<page_id_t*>(data_ + INDEX_HEADER_PGID_OFFSET);
        size_t_ column_num = *reinterpret_cast<page_id_t*>(data_ + COLUMN_NUM_OFFSET);

        // construct columns to make TableSchema
        std::vector<Column> cols;
        size_t_ col_name_len;
        offset_t offset_in_tp; // the column data's offset in tuple
        TypeId type_id;
        size_t_ data_size;

        size_t_ col_size; // record how large space this column's info occupy

        size_t_ col_name_len_offset = FIRST_COLUMN_OFFSET;
        size_t_ name_offset;
        size_t_ offset_offset; // interesting name
        size_t_ type_id_offset;
        size_t_ data_size_offset;
        for (size_t_ i = 0; i < column_num; i++) {
            col_size = FIXED_COL_SIZE;

            col_name_len = *reinterpret_cast<size_t_*>(data_ + col_name_len_offset);
            name_offset = col_name_len_offset + SIZE_T_SIZE;
            offset_offset = name_offset + col_name_len;
            offset_in_tp = *reinterpret_cast<offset_t*>(data_ + name_offset);
            type_id_offset = offset_in_tp + OFFSET_T_SIZE;
            type_id = *reinterpret_cast<TypeId*>(data_ + offset_in_tp + OFFSET_T_SIZE);
            data_size_offset = type_id_offset + ENUM_SIZE;
            data_size = *reinterpret_cast<size_t_*>(data_ + data_size_offset);

            char name[col_name_len+1];
            for (int j = 0; j < col_name_len; j++)
                name[j] = *reinterpret_cast<char*>(data_ + name_offset + j);
            name[col_name_len] = '\0';

            // make column
            if (type_id == TypeId::CHAR) {
                cols.push_back(Column(name, offset_in_tp, data_size));
            } else {
                cols.push_back(Column(type_id, name, offset_in_tp));
            }
        }

        // create TableSchema
        table_schema_ = new TableSchema(cols);
    }

} // namespace dawn
