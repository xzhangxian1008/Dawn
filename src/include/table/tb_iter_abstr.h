#pragma once

#include "table/tuple.h"

namespace dawn {

class TableIterAbstract {
public:
    virtual ~TableIterAbstract() {};
    virtual const Tuple& operator*() const = 0;
    virtual Tuple* operator->() const = 0;
    virtual TableIterAbstract &operator++() = 0;
};

} // namespace dawn
