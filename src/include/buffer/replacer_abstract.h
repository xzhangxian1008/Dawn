#pragma once

#include "util/config.h"
#include "util/util.h"

namespace dawn {

class ReplacerAbstract {
public:
    ReplacerAbstract() = default;
    virtual ~ReplacerAbstract() = default;
    virtual void pin(frame_id_t) = 0;
    virtual void unpin(frame_id_t) = 0;
    virtual void victim(frame_id_t*) = 0;
    virtual int size() = 0;
};

} // namespace dawn
