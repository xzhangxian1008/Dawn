#pragma once

#include <atomic>
#include <string.h>

#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"

namespace dawn {

/**
 * common page header layout(64 bytes):
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

    inline void w_lock() { latch_.w_lock(); }
    inline void w_unlock() { latch_.w_unlock(); }
    inline void r_lock() { latch_.r_lock(); }
    inline void r_unlock() { latch_.r_unlock(); }

    inline char *get_data() { return data_; }
    inline page_id_t get_page_id() { return page_id_; }
    inline int get_pin_count() { return pin_count_; }
    inline bool is_dirty() { return is_dirty_; }
    inline lsn_t get_lsn() { return lsn_; }

    inline void add_pin_count() { pin_count_++; }
    inline void decrease_pin_count() { pin_count_--; }
    inline void set_is_dirty(bool is_dirty) { is_dirty_ = is_dirty; }

    void set_page_id(page_id_t page_id) {
        latch_.w_lock();
        memcpy(data_ + PAGE_ID_OFFSET, &page_id, sizeof(page_id_t));
        page_id_ = page_id;
        latch_.w_unlock();
    }

    void set_lsn(lsn_t lsn) {
        latch_.w_lock();
        memcpy(data_ + LSN_OFFSET, &lsn, sizeof(lsn_t));
        lsn_ = lsn;
        latch_.w_unlock();
    }

    void set_status() {
        char s = STATUS_EXIST;
        latch_.w_lock();
        memcpy(data_ + STATUS_OFFSET, &s, sizeof(1));
        latch_.w_unlock();
    }
private:
    void reset_mem() { memset(data_, 0, PAGE_SIZE); }

    std::atomic<page_id_t> page_id_;
    std::atomic<int> pin_count_;
    std::atomic<bool> is_dirty_;
    std::atomic<lsn_t> lsn_;

    char data_[PAGE_SIZE];

    // latch_ only protects the data_
    ReaderWriterLatch latch_;
};

} // namespace dawn