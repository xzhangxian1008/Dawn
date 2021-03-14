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
    Page* new_page() { return alloc_page(STATUS_EXIST); }
    Page* new_tmp_page() { return alloc_page(STATUS_TMP); }
    Page* alloc_page(char flag);
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

    /**
     * The buffer pool manager is responsible for the allocation of the threshold_page.
     * Only 4/5 of the pool_size_ will be allow to allocated for the threshold_page, because
     * if we allocate all the pages for executors the buffer pool may be totally consumed
     * and every executor will wait for others to unpin the pages, so the dead lock will happen.
     * 
     * Allocate half of the available_threshold_page_ each time.
     * 
     * If there is no enough pages to be allocated, wait, until we can allocate.
     */
    size_t_ get_threshold_page() {
        size_t_ alloc_num = available_threshold_page_ / 2;
        while (true) {
            latch_.w_lock();
            if (alloc_num + allocated_threshold_pages_ > available_threshold_page_) {
                latch_.w_unlock();

                // wait, until we can allocate
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            } else {
                latch_.w_unlock();
                return alloc_num;
            }
        }

    }

    /** @param ret_threpage show how many pages are returned by the executor */
    void return_threshold_page(size_t_ ret_thre_page) {
        latch_.w_lock();
        allocated_threshold_pages_ -= ret_thre_page;
        latch_.w_unlock();
    }

protected:
    // ATTENTION no lock protects it
    inline frame_id_t get_frame_id(const page_id_t &page_id) const;

    // we assume this function will always success.
    void evict_page(const page_id_t &page_id, const frame_id_t &frame_id);

    ReaderWriterLatch latch_;
private:
    // a page pool
    Page *pages_;

    size_t_ pool_size_;

    size_t_ available_threshold_page_;

    size_t_ allocated_threshold_pages_;

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
