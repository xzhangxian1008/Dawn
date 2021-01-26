#include "buffer/buffer_pool_manager.h"

namespace dawn {

BufferPoolManager::BufferPoolManager(DiskManager *disk_manager, int pool_size)
    : disk_manager_(disk_manager), pool_size_(pool_size) {
    pages_ = new Page[pool_size_];
    replacer_ = new ClockReplacer(pool_size_);
    for (int i = 0; i < pool_size_; i++)
        free_list_.push_back(i);
}

Page* BufferPoolManager::new_page() {
    // get new page id
    page_id_t page_id = disk_manager_->get_new_page();
    frame_id_t frame_id;

    // ensure the free_list_ can provide a free frame id
    latch_.w_lock();
    while (free_list_.empty()) {
        latch_.w_unlock();
        replacer_->victim(&frame_id);
        evict_page(pages_[frame_id].get_page_id(), frame_id);
        latch_.w_lock();
    }

    // get a free frame id
    frame_id = free_list_.front();
    free_list_.pop_front();

    mapping_.insert(std::make_pair(page_id, frame_id));
    latch_.w_unlock();
    
    // initialize the page
    pages_[frame_id].set_is_dirty(true); // new page is always dirty
    pages_[frame_id].set_pin_count_zero();
    pages_[frame_id].add_pin_count();
    pages_[frame_id].set_lsn(-1);
    pages_[frame_id].set_page_id(page_id);
    pages_[frame_id].set_status();
    
    return &(pages_[frame_id]);
}

void BufferPoolManager::unpin_page(const page_id_t &page_id, const bool is_dirty) {
    latch_.w_lock();
    auto iter = mapping_.find(page_id);
    if (iter == mapping_.end()) {
        latch_.w_unlock();
        return;
    }

    frame_id_t frame_id = iter->second;
    pages_[frame_id].set_is_dirty(is_dirty);
    pages_[frame_id].decrease_pin_count();
    if (pages_[frame_id].get_pin_count() > 0) {
        latch_.w_unlock();
        return;
    }

    // no one need it, put him into replacer
    replacer_->unpin(frame_id);
    latch_.w_unlock();
}

bool BufferPoolManager::flush_page(const page_id_t &page_id) {
    latch_.r_lock();
    frame_id_t frame_id = get_frame_id(page_id);
    latch_.r_unlock();

    // page id should always be found, here just in case
    if (frame_id == -1) {
        return false;
    }

    // flush dirty page and reset it's dirty flag
    char *data = pages_[frame_id].get_data();
    pages_[frame_id].w_lock();
    
    if (!disk_manager_->write_page(page_id, data)) {
        pages_[frame_id].w_unlock();
        return false;
    }

    pages_[frame_id].w_unlock();
    pages_[frame_id].set_is_dirty(false);
    return true;
}

void BufferPoolManager::flush_all() {
    
}

void BufferPoolManager::evict_page(const page_id_t &page_id, const frame_id_t &frame_id) {
    pages_[frame_id].w_lock();
    latch_.w_lock();
    auto iter = mapping_.find(page_id);
    if (iter == mapping_.end()) {
        pages_[frame_id].w_unlock();
        latch_.w_unlock();
        return;
    }

    // update meta data
    mapping_.erase(iter);
    free_list_.push_back(frame_id);

    // if the page is dirty, flush it first
    latch_.w_unlock();
    if (pages_[frame_id].is_dirty()) {
        disk_manager_->write_page(page_id, pages_[frame_id].get_data());
    }
    pages_[frame_id].w_unlock();
}

/**
 * Two situation:
 *   1. get an existing page
 *   2. fetch a page from disk
 */
Page* BufferPoolManager::get_page(const page_id_t &page_id) {
    // firstly, judge we are in which situation
    latch_.w_lock();
    auto iter = mapping_.find(page_id);
    if (iter != mapping_.end()) {
        // situation 1
        pages_[iter->second].add_pin_count();
        latch_.w_unlock();
        return &(pages_[iter->second]);
    } else {
        // situation 2, ensure free frame id is available
        frame_id_t frame_id;
        while (free_list_.empty()) {
            latch_.w_unlock();
            replacer_->victim(&frame_id);
            evict_page(page_id, frame_id);
            latch_.w_lock();
        }

        // get a free frame id
        frame_id = free_list_.front();

        // read page from disk
        if (!disk_manager_->read_page(page_id, pages_[frame_id].get_data())) {
            LOG("read page fail");
            return nullptr;
        }

        // check if this is a valid page
        char *data = pages_[frame_id].get_data();
        if (*reinterpret_cast<char*>(data + STATUS_OFFSET) != STATUS_EXIST) {
            latch_.w_unlock();
            LOG("get a invalid page");
            return nullptr;
        }

        // initialize the page
        pages_[frame_id].set_is_dirty(false);
        pages_[frame_id].set_page_id(page_id);
        pages_[frame_id].set_lsn(-1);
        pages_[frame_id].set_pin_count_zero();
        pages_[frame_id].add_pin_count();

        // update meta data
        mapping_.insert(std::make_pair(page_id, frame_id));
        free_list_.pop_front();

        latch_.w_unlock();
        return &(pages_[frame_id]);
    }
}

/**
 * if someone is using the page, return false.
 */
bool BufferPoolManager::delete_page(const page_id_t &page_id) {
    // find if this page exists in the buffer pool
    latch_.w_lock();
    auto iter = mapping_.find(page_id);
    if (iter == mapping_.end()) {
        // page is not in the buffer pool, free it immediately
        latch_.w_unlock();
        disk_manager_->free_page(page_id); // suppose it always successes
        return true;
    }

    // page is in the buffer pool, check if someone else is using it
    if (pages_[iter->second].get_pin_count() != 0) {
        // someone else is using it
        latch_.w_unlock();
        return false;
    }

    // update meta data
    free_list_.push_back(iter->second);
    mapping_.erase(iter);

    latch_.w_unlock();

    // delete it immediately and we suppose it always successes
    disk_manager_->free_page(page_id);
    return true;
}

} // namespace dawn
