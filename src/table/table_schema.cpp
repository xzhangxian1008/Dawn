#include "table/table_schema.h"

namespace dawn {

TableSchema* create_table_schema(std::vector<TypeId> types, std::vector<string_t> names, std::vector<size_t_> name_len) {
    if (types.size() != names.size())
        return nullptr;
    
    offset_t offset = 0;
    std::vector<Column> cols;
    size_t j = 0;
    for (size_t i = 0; i < types.size(); i++) {
        if (types[i] == TypeId::CHAR) {
            cols.push_back(Column(names[i], offset, name_len[j]));
            offset += name_len[j++];
            continue;
        }
        cols.push_back(Column(types[i], names[i], offset));
        offset += Type::get_type_size(types[i]);
    }

    return new TableSchema(cols);
}

} // namespace dawn
