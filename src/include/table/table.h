#pragma once

#include "buffer/buffer_pool_manager.h"
#include "storage/page/page.h"
#include "storage/page/table_page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"

namespace dawn {

class Table {
public:
    // if from_scratch == true, it means that the Table should initialize the page in the disk
    Table(BufferPoolManager *bpm, const page_id_t first_table_page_id, bool from_scratch = false);
    // TODO a lot of operation about tuple's crud
private:
    BufferPoolManager *bpm_;
    const page_id_t first_table_page_id_;
    ReaderWriterLatch latch_;
};

} // namespace dawn
