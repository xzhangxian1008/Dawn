#pragma once

#include "table/tb_iter_abstr.h"
#include "buffer/buffer_pool_manager.h"
#include "storage/page/link_hash_page.h"

namespace dawn {

class LinkHashTableIter : public TableIterAbstract {
public:
    /** initialize the iter from the beginning */
    LinkHashTableIter(page_id_t first_page_id, BufferPoolManager *bpm);

    /** initialize the iter from the designated position */
    LinkHashTableIter(page_id_t first_page_id, BufferPoolManager *bpm, RID cur_rid)
        : first_page_id_(first_page_id), bpm_(bpm) {
        new Tuple();
    }

    ~LinkHashTableIter() override { delete tuple_; }

    const Tuple& operator*() override { return *tuple_; }

    Tuple* operator->() override { return tuple_; }

    TableIterAbstract &operator++() override;
private:
    page_id_t first_page_id_;
    BufferPoolManager *bpm_;
    Tuple *tuple_; // store the tuple referred by the iter
};

} // namespace dawn
