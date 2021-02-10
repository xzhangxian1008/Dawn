#pragma once

#include "table/tuple.h"

namespace dawn {

class TableIterAbstract {
public:
    virtual ~TableIterAbstract() = 0;
    virtual const Tuple& operator*() = 0;
    virtual Tuple* operator->() = 0;
    virtual TableIterAbstract &operator++() = 0;
};

} // namespace dawn
