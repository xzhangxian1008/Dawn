#pragma once

#include <sstream>

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "data/types.h"

namespace dawn {

class Column {
public:
    explicit Column(const TypeId column_type, const string_t &column_name, offset_t offset)
        : column_type_(column_type), column_name_(column_name), offset_(offset), fixed_length_(get_type_size(column_type)) {}

    // for the char initialization
    explicit Column(const string_t &column_name, offset_t offset, size_t_ char_length)
        : column_name_(column_name), offset_(offset), char_length_(char_length), fixed_length_(PTR_SIZE), column_type_(TypeId::kChar) {}

    ~Column() = default;

    inline offset_t get_offset() const { return offset_; }
    inline TypeId get_type_id() const { return column_type_; }
    inline string_t get_column_name() const { return column_name_; }

    inline size_t_ get_data_size() const {
        if (column_type_ != TypeId::kChar)
            return fixed_length_;
        else if (column_type_ == TypeId::kChar)
            return char_length_;
        
        return -1;
    }

    // FIXME I think it's a bad method
    string_t to_string() const {
        std::ostringstream os;

        os << "Column[" << column_name_ << ", " << type_to_string(column_type_) << ", "
            << "offset:" << offset_ << ", ";

        if (column_type_ != TypeId::kChar) {
            os << "fixed length:" << fixed_length_;
        } else {
            os << "char length:" << char_length_;
        }
        os << "]";
        return (os.str());
    }

    // the equal here means that every field in the column should equal
    static bool is_columns_equal(const Column &col1, const Column &col2) {
        if (col1.column_type_ != col2.column_type_) {
            PRINT("column type not equal");
            PRINT(type_to_string(col1.column_type_), type_to_string(col2.column_type_));
            return false;
        }
        if (col1.column_name_ != col2.column_name_) {
            PRINT("column name not equal");
            PRINT(col1.column_name_, col2.column_name_);
            return false;
        }
        if (col1.offset_ != col2.offset_) {
            PRINT("column offset not equal");
            PRINT(col1.offset_, col2.offset_);
            return false;
        }
        if (col1.get_data_size() != col2.get_data_size()) {
            return false;
        }
        return true;
    }

private:
    size_t_ get_type_size(TypeId type_id) {
        switch (type_id) {
            case TypeId::kInvalid:
                return -1;
            case TypeId::kBoolean:
                return Type::get_bool_size();
            case TypeId::kInteger:
                return Type::get_integer_size();
            case TypeId::kChar:
                return char_length_;
            case TypeId::kDecimal:
                return Type::get_decimal_size();
        }

        PRINT("WARNING: Invalid TypeId");
        return -1;
    }

    TypeId column_type_;
    string_t column_name_;
    offset_t offset_; // offset in the tuple
    size_t_ fixed_length_; // data length of the column
    size_t_ char_length_; // do not contain the '\0', and tuple not contains it as well
};

} // namespace dawn