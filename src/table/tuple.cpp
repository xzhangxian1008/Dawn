#include "table/tuple.h"

namespace dawn {
    
Tuple::Tuple(std::vector<Value> &values, const TableSchema &schema) {
    size_ = schema.get_tuple_size();
    allocated_ = true;
    data_ = new char[size_];

    size_t_ col_num = schema.get_column_num();
    for (size_t_ i = 0; i < col_num; i++) {
        Column col = schema.get_column(i);
        values[i].serialize_to(data_ + col.get_offset());
    }
}

Value Tuple::get_value(const TableSchema &schema, int idx) {
    Column col = schema.get_column(idx);
    TypeId type_id = col.get_type_id();
    return Value(data_ + col.get_offset(), type_id);
}

void Tuple::set_value(const TableSchema &schema, Value &value, int idx) {
    Column col = schema.get_column(idx);
    value.serialize_to(data_ + col.get_offset());
}

string_t Tuple::to_string(const TableSchema *schema) {
    
}

} // namespace dawn
