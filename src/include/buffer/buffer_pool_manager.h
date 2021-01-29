#pragma once

#include <unordered_map>
#include <deque>

#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "storage/page/page.h"
#include "storage/disk/disk_manager.h"
#include "buffer/clock_replacer.h"

namespace dawn {

/**
 * FIXME logging and transaction will be added in the future
 */
class BufferPoolManager {
public:
    explicit BufferPoolManager(DiskManager *disk_manager, int pool_size);

    ~BufferPoolManager() {
        delete[] pages_;
        delete replacer_;
    }

    Page* get_page(const page_id_t &page_id);
    Page* new_page();
    void unpin_page(const page_id_t &page_id, const bool is_dirty);
    bool flush_page(const page_id_t &page_id);

    /**
     * @return false if flush some pages fail
     */
    bool flush_all();

    /**
     * @return false if someone is using it and it's caller's duty to decide what to do next
     */
    bool delete_page(const page_id_t &page_id);

protected:
    // ATTENTION no lock protects it
    inline frame_id_t get_frame_id(const page_id_t &page_id) const;

    // we assume this function will always success.
    void evict_page(const page_id_t &page_id, const frame_id_t &frame_id);

    size_t_ get_size() { return replacer_->size(); } // delete it

    ReaderWriterLatch latch_;
private:
    // a page pool
    Page *pages_;

    int pool_size_;

    std::deque<frame_id_t> free_list_;

    // map the page id to frame id
    std::unordered_map<page_id_t, frame_id_t> mapping_;

    DiskManager *disk_manager_;

    ReplacerAbstract *replacer_;
};

inline frame_id_t BufferPoolManager::get_frame_id(const page_id_t &page_id) const {
    auto iter = mapping_.find(page_id);
    if (iter == mapping_.end()) {
        return -1;
    }
    return iter->second;
}

} // namespace dawn
