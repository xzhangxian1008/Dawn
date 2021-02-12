#include "table/lk_ha_tb_iter.h"

namespace dawn {
LinkHashTableIter::LinkHashTableIter(page_id_t first_page_id, BufferPoolManager *bpm) 
    : first_page_id_(first_page_id), bpm_(bpm) {
    new Tuple();
    
    LinkHashPage *lk_page = reinterpret_cast<LinkHashPage*>(bpm_->get_page(first_page_id_));
    offset_t slot_num = -1;
    
    while (++slot_num < LK_HA_PG_SLOT_NUM) {
        /** 
         * find the first non-empty slot, but the first non-empty slot may not 
         * contain a tuple. So we should find another non-empty until we can get or not.
         */
        lk_page->r_lock();
        for (; slot_num < LK_HA_PG_SLOT_NUM; slot_num++) {
            page_id_t page_id = lk_page->get_pgid_in_slot(slot_num);
            if (page_id != INVALID_PAGE_ID) {
                lk_page->r_unlock();
                bpm_->unpin_page(lk_page->get_page_id(), false);
                lk_page = reinterpret_cast<LinkHashPage*>(bpm_->get_page(page_id));
                break;
            }
        }

        if (slot_num == LK_HA_PG_SLOT_NUM) {
            tuple_->set_rid(RID(INVALID_PAGE_ID, INVALID_SLOT_NUM));
        }

        lk_page->r_lock(); // this lk_page has been change in the above for-loop
    }

    // TODO
}

TableIterAbstract& LinkHashTableIter::operator++() {
    // TODO need imple
    return *this;
}

} // namespace dawn
