#include "data/types.h"

namespace dawn {

string_t type_to_string(TypeId type_id) {
    switch (type_id) {
        case TypeId::kInvalid:
            return "kInvalid";
        case TypeId::kBoolean:
            return "kBoolean";
        case TypeId::kInteger:
            return "kInteger";
        case TypeId::kDecimal:
            return "kDecimal";
        case TypeId::kChar:
            return "kChar";
    }
    return "ILLEGAL";
}

} // namespace dawn
