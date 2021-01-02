#pragma once

#include <atomic>
#include <string.h>

#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"

namespace dawn {

/**
 * page common header layout(64 bytes):
 * -------------------------------------------------------------
 * | Status (1) | LSN (4) | PageId (4) |      Reserved (55)    |
 * -------------------------------------------------------------
 */
class Page {
public:
    Page(page_id_t page_id, char *data_src = nullptr) {
        page_id_ = page_id;
        pin_count_ = 0;
        is_dirty_ = false;
        reset_mem();

        if (data_src != nullptr)
            memcpy(data_, data_src, PAGE_SIZE);
        
        char *status = reinterpret_cast<char*>(data_);
        *status = STATUS_EXIST;
        lsn_t *lt = reinterpret_cast<lsn_t*>(data_ + LSN_OFFSET);
        *lt = -1;
        page_id_t *pt = reinterpret_cast<page_id_t*>(data_ + PAGE_ID_OFFSET);
        *pt = page_id;
    }

    Page() {
        page_id_ = INVALID_PAGE_ID;
        pin_count_ = 0;
        is_dirty_ = 0;
        reset_mem();
    }

    char *get_data() { return data_; }
    page_id_t get_page_id() { return page_id_; }
    int get_pin_count() { return pin_count_; }
    bool is_dirty() { return is_dirty_; }
    void w_latch() { rwlatch_.w_lock(); }
    void w_unlatch() { rwlatch_.w_unlock(); }
    void r_latch() { rwlatch_.r_lock(); }
    void r_unlatch() { rwlatch_.r_unlock(); }
    lsn_t get_lsn() { return *reinterpret_cast<lsn_t *>(data_ + LSN_OFFSET); }
    void set_lsn(lsn_t lsn) { memcpy(data_ + LSN_OFFSET, &lsn, sizeof(lsn_t)); }

private:
    void reset_mem() { memset(data_, 0, PAGE_SIZE); }

    char data_[PAGE_SIZE];
    std::atomic<page_id_t> page_id_;
    std::atomic<int> pin_count_;
    std::atomic<bool> is_dirty_;
    ReaderWriterLatch rwlatch_;
};

} // namespace dawn