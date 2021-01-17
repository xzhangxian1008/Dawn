#include "data/types.h"

namespace dawn {

string_t type_to_string(TypeId type_id) {
    switch (type_id) {
        case TypeId::INVALID:
            return "INVALID";
        case TypeId::BOOLEAN:
            return "BOOLEAN";
        case TypeId::INTEGER:
            return "INTEGER";
        case TypeId::DECIMAL:
            return "DECIMAL";
        case TypeId::CHAR:
            return "CHAR";
    }
    return "ILLEGAL";
}

} // namespace dawn
