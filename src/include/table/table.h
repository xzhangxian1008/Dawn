#pragma once

#include "buffer/buffer_pool_manager.h"
#include "storage/page/page.h"
#include "storage/page/table_page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/rid.h"
#include "table/tuple.h"
#include "table/rid.h"

namespace dawn {

/**
 * ATTENTION problem
 * delete、update and get tuples could be done with the RID parameter directly.
 * Is it safe? How can I ensure they are legal RID and won't do some strange operation?
 * Do we need to maintain another data structure to ensure the RID are legal?
 */
class Table {
public:
    // if from_scratch == true, it means that the Table should initialize the page in the disk
    Table(BufferPoolManager *bpm, const page_id_t first_table_page_id, bool from_scratch = false);
    ~Table() = default;
    void delete_all_data();
    page_id_t get_first_table_page_id() const { return first_table_page_id_; }

    bool insert_tuple(const Tuple &tuple, RID *rid);
    bool mark_delete(const RID &rid);
    void apply_delete(const RID &rid);
    void rollback_delete(const RID &rid);
    bool get_tuple(Tuple *tuple, const RID &rid);
    bool update_tuple(const Tuple &tuple, const RID &rid);
private:
    BufferPoolManager *bpm_;
    const page_id_t first_table_page_id_;
    ReaderWriterLatch latch_;
};

} // namespace dawn
