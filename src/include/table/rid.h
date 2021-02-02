#pragma once

#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"

namespace dawn {

/**
 * ATTENTION no lock!
 */
class RID {
public:
    explicit RID(page_id_t page_id, offset_t slot_num) : page_id_(page_id), slot_num_(slot_num) {}
    RID() : page_id_(INVALID_PAGE_ID), slot_num_(INVALID_SLOT_NUM) {}

    inline page_id_t get_page_id() const { return page_id_; }
    inline offset_t get_slot_num() const { return slot_num_; }

    void set(page_id_t page_id, offset_t slot_num) {
        page_id_ = page_id;
        slot_num_ = slot_num;
    }

    bool operator==(const RID &rid) { return (this->page_id_ == rid.page_id_ ) && (this->slot_num_ == rid.slot_num_); }

private:
    page_id_t page_id_;
    offset_t slot_num_;
};

} // namespace dawn
