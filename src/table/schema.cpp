#include "table/schema.h"

namespace dawn {

Schema* create_table_schema(const std::vector<TypeId> &types, const std::vector<string_t> &names, const std::vector<size_t_> &char_len) {
    if (types.size() != names.size())
        return nullptr;
    
    offset_t offset = 0;
    std::vector<Column> cols;
    size_t j = 0;
    for (size_t i = 0; i < types.size(); i++) {
        if (types[i] == TypeId::kChar) {
            cols.push_back(Column(names[i], offset, char_len[j]));
            offset += char_len[j++];
            continue;
        }
        cols.push_back(Column(types[i], names[i], offset));
        offset += Type::get_type_size(types[i]);
    }

    return new Schema(cols);
}

bool is_table_schemas_equal(const Schema &ts1, const Schema &ts2) {
    std::vector<Column> cols1 = ts1.get_columns();
    std::vector<Column> cols2 = ts2.get_columns();
    size_t col_num = cols1.size();
    if (col_num != cols2.size()) {
        PRINT("size not equal:", col_num, cols2.size());
        return false;
    }

    if (ts1.get_tuple_size() != ts2.get_tuple_size())
        return false;

    for (size_t i = 0; i < col_num; i++) {
        if (Column::is_columns_equal(cols1[i], cols2[i])) {
            continue;
        }
        PRINT("not equal, column name:", cols1[i].get_column_name(), "i:", i);
        return false;
    }
    return true;
}

} // namespace dawn
