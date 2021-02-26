#pragma once

#include "data/values.h"
#include "table/schema.h"
#include "table/tuple.h"

namespace dawn {

class ExpressionAbstract {
public:
    ExpressionAbstract() = default;
    virtual ~ExpressionAbstract() = default;
    virtual Value Evaluate(const Tuple *tuple, const Schema *schema) = 0;
private:

};

} // namespace dawn
