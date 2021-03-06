#pragma once

#include "table/tb_iter_abstr.h"
#include "buffer/buffer_pool_manager.h"
#include "storage/page/link_hash_page.h"
#include "storage/page/table_page.h"

namespace dawn {

class LinkHashTableIter : public TableIterAbstract {
public:
    /** initialize the iter from the beginning */
    LinkHashTableIter(page_id_t first_page_id, BufferPoolManager *bpm);

    ~LinkHashTableIter() override { delete tuple_; }

    const Tuple& operator*() const override { return *tuple_; }

    Tuple* operator->() const override { return tuple_; }

    TableIterAbstract &operator++() override;
private:
    page_id_t first_page_id_;
    BufferPoolManager *bpm_;
    Tuple *tuple_; // store the tuple referred by the iter

    /** 
     * refer to the first level page's slot,
     * help the iter to know it accesses which second level page
     */
    offset_t slot1_num_; 

    /** 
     * refer to the second level page's slot,
     * help the iter to know it accesses which link list
     */
    offset_t slot2_num_;

    /**
     * Refer to the TablePage which the tuple stays on.
     * Always holds the write lock and pins the page.
     * nullptr, if the iter reaches to the end
     */
    TablePage *cur_tb_page_;
};

} // namespace dawn
