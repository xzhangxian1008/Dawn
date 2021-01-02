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
    page_id_t page_id = disk_manager_->get_new_page();
    frame_id_t frame_id;

    latch_.w_lock();
    while (free_list_.size() == 0) {
        latch_.w_unlock();
        replacer_->victim(&frame_id);
        evict_page(page_id, frame_id);
        latch_.w_lock();
    }

    frame_id = free_list_.front();
    free_list_.pop_front();

    // initialize the page
    pages_[frame_id].set_is_dirty(false);
    pages_[frame_id].add_pin_count();
    pages_[frame_id].set_lsn(-1);
    pages_[frame_id].set_page_id(page_id);
    pages_[frame_id].set_status();
    latch_.w_unlock();

    return &(pages_[frame_id]);
}

inline frame_id_t BufferPoolManager::get_frame_id(page_id_t page_id) {
    auto iter = mapping_.find(page_id);
    if (iter == mapping_.end()) {
        return -1;
    }
    return iter->second;
}

void BufferPoolManager::unpin_page(page_id_t page_id) {
    latch_.w_lock();
    auto iter = mapping_.find(page_id);
    if (iter == mapping_.end()) {
        latch_.w_unlock();
        return;
    }

    frame_id_t frame_id = iter->second;
    pages_[frame_id].decrease_pin_count();
    if (pages_[frame_id].get_pin_count() != 0) {
        latch_.w_unlock();
        return;
    }

    // no one need it, put him into replacer
    replacer_->unpin(frame_id);
    latch_.w_unlock();
}

bool BufferPoolManager::flush_page(page_id_t page_id) {
    latch_.r_lock();
    frame_id_t frame_id = get_frame_id(page_id);
    if (frame_id == -1) {
        latch_.r_unlock();
        return false;
    }
    if (!disk_manager_->write_page(page_id, pages_[page_id].get_data())) {
        latch_.r_unlock();
        return false;
    }
    latch_.r_unlock();
    return true;
}

void BufferPoolManager::evict_page(page_id_t page_id, frame_id_t frame_id) {
    pages_[frame_id].w_lock();
    latch_.w_lock();
    auto iter = mapping_.find(page_id);
    mapping_.erase(iter);
    free_list_.push_back(frame_id);

    latch_.w_unlock();
    if (pages_[frame_id].is_dirty()) {
        disk_manager_->write_page(page_id, pages_[frame_id].get_data());
    }
    pages_[frame_id].w_unlock();
}

// TODO
Page* BufferPoolManager::get_page(page_id_t page_id) {

}

// TODO
void BufferPoolManager::delete_page(page_id_t page_id) {

}

} // namespace dawn
