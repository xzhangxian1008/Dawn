#include "buffer/buffer_pool_manager.h"

namespace dawn {

BufferPoolManager::BufferPoolManager(DiskManager *disk_manager, int pool_size)
    : disk_manager_(disk_manager), pool_size_(pool_size) {
    pages_ = new Page[pool_size_];
    replacer_ = new ClockReplacer(pool_size_);
    for (int i = 0; i < pool_size_; i++)
        free_list_.push_back(i);
}



} // namespace dawn
