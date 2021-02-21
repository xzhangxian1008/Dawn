#pragma once

#include <vector>

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/column.h"

namespace dawn {

/**
 * TODO set the key index
 * ATTENTION no latch to handle the concurrency environment, because we won't modify TableSchema after it is created
 * for specification of the tuple
 */
class TableSchema {
public:
    explicit TableSchema(const std::vector<Column> &columns) : columns_(columns), length_(0) {
        for (auto &col : columns_)
            length_ += col.get_data_size();
    }

    ~TableSchema() = default;

    inline size_t_ get_tuple_size() const { return length_; }
    inline int get_column_num() const { return columns_.size(); }

    // QUESTION who should ensure the index is valid?
    inline Column get_column(int index) const { return columns_[index]; }
    inline std::vector<Column> get_columns() const { return columns_; }
    inline string_t get_column_name(int index) const { return columns_[index].get_column_name(); }
    inline TypeId get_column_type(int index) const { return columns_[index].get_type_id(); }
    inline offset_t get_column_offset(int index) const { return columns_[index].get_offset(); }
    inline size_t_ get_column_size(int index) const { return columns_[index].get_data_size(); }

    int get_column_idx(const string_t &column_name) const {
        for (int i = 0; i < get_column_num(); i++)
            if (columns_[i].get_column_name() == column_name)
                return i;
        
        return -1;
    }

    inline offset_t get_key_idx() const {
        return key_idx_;
    }

    inline void set_key_idx(offset_t key_idx) {
        key_idx_ = key_idx;
    }

    // FIXME I think it's a bad method
    string_t to_string() const {
        std::ostringstream os;

        os << "Schema["
            << "column number:" << get_column_num() << ", "
            << "tuple size:" << length_ << "]";

        bool first = true;
        os << " :: (";
        for (int i = 0; i < get_column_num(); i++) {
            if (first) {
            first = false;
            } else {
            os << ", ";
            }
            os << columns_[i].to_string();
        }
        os << ")";

        return os.str();
    }

private:
    std::vector<Column> columns_;
    size_t_ length_; // the tuple's size

    // TODO the default key index is 0, change it!
    offset_t key_idx_ = 0; // key index, and we do not support the jointly index
};

TableSchema* create_table_schema(const std::vector<TypeId> &types, 
    const std::vector<string_t> &names, const std::vector<size_t_> &char_len = std::vector<size_t_>{});

bool is_table_schemas_equal(const TableSchema &ts1, const TableSchema &ts2);

} // namespace dawn