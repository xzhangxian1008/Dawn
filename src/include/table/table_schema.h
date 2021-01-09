#pragma once

#include <vector>

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/column.h"

namespace dawn {

/**
 * for specification of the tuple
 */
class TableSchema {
public:
    TableSchema(const std::vector<Column> &columns) : columns_(columns), length_(0) {
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

    // FIXME I think it's a bad method
    string_t to_string() const {
        std::ostringstream os;

        os << "Schema["
            << "column number:" << get_column_num() << ", "
            << "tuple size:" << length_ << "]";

        bool first = true;
        os << " :: (";
        for (uint32_t i = 0; i < get_column_num(); i++) {
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
};

} // namespace dawn