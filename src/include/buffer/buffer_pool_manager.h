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
        delete pages_;
        delete replacer_;
    }

private:
    // a page pool
    Page *pages_;

    int pool_size_;

    std::deque<frame_id_t> free_list_;

    // map the page id to frame id
    std::unordered_map<page_id_t, frame_id_t> mapping_;

    DiskManager *disk_manager_;

    ReplacerAbstract *replacer_;

    ReaderWriterLatch latch_;
};

} // namespace dawn
