#include "table/tuple.h"

namespace dawn {

Value Tuple::get_value(const TableSchema &schema, int idx) const {
    Column col = schema.get_column(idx);
    TypeId type_id = col.get_type_id();
    if (type_id == TypeId::CHAR) {
        size_t_ str_size = col.get_data_size();

        // we have to create a temporary string, because the Tuple and string stored on the disk doesn't end with '\0'
        char tmp[str_size+1];
        memcpy(tmp, data_ + col.get_offset(), str_size);
        tmp[str_size] = '\0';
        return Value(tmp, type_id, str_size);
    }
    return Value(data_ + col.get_offset(), type_id);
}

// ATTENTION length of string is not checked here
void Tuple::set_value(const TableSchema &schema, Value *value, int idx) {
    Column col = schema.get_column(idx);
    if (col.get_type_id() == TypeId::CHAR) {
        size_t_ str_size = col.get_data_size();
        // we have to create a temporary string, because the string stored on the disk and Tupe doesn't end with '\0'
        char tmp[str_size+1];
        value->serialize_to(tmp);
        memcpy(data_ + col.get_offset(), tmp, str_size);
        return;
    }
    value->serialize_to(data_ + col.get_offset());
}

string_t Tuple::to_string(const TableSchema &schema) const {
    std::ostringstream os;
    int col_num = schema.get_column_num();

    os << "Tuple: [ column number: " << col_num << ", tuple size: "
        << schema.get_tuple_size() << ", (";
    
    bool first = true;
    for (int i = 0; i < col_num; i++) {
        Column col = schema.get_column(i);

        if (!first) {
            os << ", ";
        } else {
            first = false;
        }

        Value value = get_value(schema, i);
        os << type_to_string(col.get_type_id()) << ": " << value.get_value_string();
    }

    os << ")]";
    return os.str();
}

void Tuple::init(std::vector<Value> *values, const TableSchema &schema) {
    size_ = schema.get_tuple_size();
    allocated_ = true;
    data_ = new char[size_];

    size_t_ col_num = schema.get_column_num();
    for (size_t_ i = 0; i < col_num; i++) {
        set_value(schema, &((*values)[i]), i);
    }
}

bool Tuple::operator==(const Tuple &tuple) {
    if (allocated_ != tuple.allocated_) {
        return false;
    }

    if (size_ != tuple.size_) {
        return false;
    }

    if (allocated_) {
        for (size_t_ i = 0; i < size_; i++)
            if (data_[i] != tuple.data_[i]) {
                return false;
            }
    }

    if (rid_ == tuple.rid_)
        return true;
    return false;
}

} // namespace dawn
