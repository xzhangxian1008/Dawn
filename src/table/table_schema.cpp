#include "table/table_schema.h"

namespace dawn {

TableSchema* create_table_schema(std::vector<TypeId> types, std::vector<string_t> names, std::vector<size_t_> char_len) {
    if (types.size() != names.size())
        return nullptr;
    
    offset_t offset = 0;
    std::vector<Column> cols;
    size_t j = 0;
    for (size_t i = 0; i < types.size(); i++) {
        if (types[i] == TypeId::CHAR) {
            cols.push_back(Column(names[i], offset, char_len[j]));
            offset += char_len[j++];
            continue;
        }
        cols.push_back(Column(types[i], names[i], offset));
        offset += Type::get_type_size(types[i]);
    }

    return new TableSchema(cols);
}

bool is_table_schemas_equal(const TableSchema &ts1, const TableSchema &ts2) {
    std::vector<Column> cols1 = ts1.get_columns();
    std::vector<Column> cols2 = ts2.get_columns();
    size_t col_num = cols1.size();
    if (col_num != cols2.size()) {
        PRINT("size not equal:", col_num, cols2.size());
        return false;
    }
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
