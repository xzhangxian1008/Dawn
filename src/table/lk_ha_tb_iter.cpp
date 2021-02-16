#include "table/lk_ha_tb_iter.h"
#include "storage/page/table_page.h"
#include "table/table_schema.h"

namespace dawn {

LinkHashTableIter::LinkHashTableIter(page_id_t first_page_id, BufferPoolManager *bpm) 
    : first_page_id_(first_page_id), bpm_(bpm) {

    tuple_ = new Tuple();
    
    LinkHashPage *level1_page = reinterpret_cast<LinkHashPage*>(bpm_->get_page(first_page_id_));
    offset_t slot1_num = -1; // refer to the first level page's slot
    
    /**
     * set the iterator's position to the first tuple's position
     * 
     * "while (++slot_num < LK_HA_PG_SLOT_NUM)" traverses the first level page's slot to get the
     * second level page's page id.
     * 
     * "for (; slot_num < LK_HA_PG_SLOT_NUM; slot_num++)" traverses the second level page's slot to
     * get the third level page's page id.
     */
    level1_page->r_lock();
    while (++slot1_num < LK_HA_PG_SLOT_NUM) {
        // find an available second level page
        page_id_t sec_page_id = level1_page->get_pgid_in_slot(slot1_num);
        if (sec_page_id == INVALID_PAGE_ID) {
            // unavailable second level page, carry on...
            continue;
        }

        LinkHashPage *level2_page = reinterpret_cast<LinkHashPage*>(bpm_->get_page(sec_page_id));

        // traverse the second level page to find an available third level page
        page_id_t level3_page_id = INVALID_PAGE_ID;
        level2_page->r_lock();
        offset_t slot2_num = 0;
        for (; slot2_num < LK_HA_PG_SLOT_NUM; slot2_num++) {
            level3_page_id = level2_page->get_pgid_in_slot(slot2_num);
            if (level3_page_id != INVALID_PAGE_ID)
                break;
        }
        level2_page->r_unlock();

        // find an available third level page
        if (level3_page_id != INVALID_PAGE_ID) {
            TablePage *level3_page = reinterpret_cast<TablePage*>(bpm_->get_page(level3_page_id));
            level3_page->r_lock();
            if (!level3_page->get_the_first_tuple(tuple_)) {
                // the first page of the third level should always be non-empty
                // It's a bug if we get an empty TablePage
                LOG("should not reach here");
                exit(-1);
            }
            level3_page->r_unlock();
            slot1_num_ = slot1_num;
            slot2_num_ = slot2_num;
            break;
        }

        /** 
         * can't find an available third level page, 
         * back to the head of the while-loop and
         * jumps to the next second level page
         */
    }
    level1_page->r_unlock();

    // can't find any tuple
    if (slot1_num >= LK_HA_PG_SLOT_NUM) {
        tuple_->set_rid(RID(INVALID_PAGE_ID, INVALID_SLOT_NUM));
        slot1_num_ = INVALID_SLOT_NUM;
        return;
    }
}

TableIterAbstract& LinkHashTableIter::operator++() {
    /**
     * 1. try to get next tuple in the TablePage
     * 2. try to get next tuple in the link list
     * 3. try to get next tuple by traversing the second level pages
     */
    RID cur_rid = tuple_->get_rid();
    page_id_t cur_tb_pgid = cur_rid.get_page_id();
    if (cur_tb_pgid == INVALID_PAGE_ID) {
        tuple_->set_rid(RID(INVALID_PAGE_ID, INVALID_SLOT_NUM));
        return *this;
    }
    // PRINT("position: ", slot1_num_, " ", slot2_num_);

    LinkHashPage *level1_page = reinterpret_cast<LinkHashPage*>(bpm_->get_page(first_page_id_));
    level1_page->r_lock();
    
    offset_t cur_slot1_num = slot1_num_;
    offset_t cur_slot2_num = slot2_num_;
    page_id_t cur_level2_pgid = level1_page->get_pgid_in_slot(cur_slot1_num);

    do {
        // get the second level page
        LinkHashPage *level2_page = reinterpret_cast<LinkHashPage*>(bpm_->get_page(cur_level2_pgid));
        level2_page->r_lock();

        /**
         * Need this judgement after switching to the next second level page,
         * because we should find the first available TablePage.
         */
        if (cur_tb_pgid == INVALID_PAGE_ID) {
            while (cur_slot2_num < LK_HA_PG_SLOT_NUM && cur_tb_pgid == INVALID_PAGE_ID)
                cur_tb_pgid = level2_page->get_pgid_in_slot(++cur_slot2_num);
        }

        do {
            // jump out this second level page's loop because it contains nothing
            if (cur_tb_pgid == INVALID_PAGE_ID) 
                break;

            do {
                // get next tuple in the TablePage
                TablePage *cur_tb_page = reinterpret_cast<TablePage*>(bpm_->get_page(cur_tb_pgid));
                cur_tb_page->r_lock();
                
                RID next_rid;
                bool ok;
                if (cur_rid.get_page_id() == cur_tb_page->get_page_id()) {
                    ok = cur_tb_page->get_next_tuple_rid(cur_rid, &next_rid);
                    cur_tb_page->get_tuple(tuple_, next_rid);
                } else {
                    ok = cur_tb_page->get_the_first_tuple(tuple_);
                }

                if (ok) {
                    // release some resource
                    cur_tb_page->r_unlock();
                    bpm_->unpin_page(cur_tb_pgid, false);
                    level2_page->r_unlock();
                    bpm_->unpin_page(level2_page->get_page_id(), false);
                    level1_page->r_unlock();
                    bpm_->unpin_page(first_page_id_, false);

                    slot2_num_ = cur_slot2_num;
                    slot1_num_ = cur_slot1_num;

                    return *this;
                }

                cur_tb_page->r_unlock();
                bpm_->unpin_page(cur_tb_pgid, false);

                // it's the last tuple in the TablePage, jump to the next TablePage in link list
                cur_tb_pgid = cur_tb_page->get_next_page_id();
            } while (cur_tb_pgid != INVALID_PAGE_ID); // loop in the link list

            // find the next link list containing tuples in the same second level page
            do {
                cur_tb_pgid = level2_page->get_pgid_in_slot(++cur_slot2_num);
            } while (cur_slot2_num < LK_HA_PG_SLOT_NUM && cur_tb_pgid == INVALID_PAGE_ID);

        } while (cur_slot2_num < LK_HA_PG_SLOT_NUM); // loop in an second level page

        // release the second level page's resource
        level2_page->r_unlock();
        bpm_->unpin_page(level2_page->get_page_id(), false);
        
        // find the next available second level page
        do {
            cur_level2_pgid = level1_page->get_pgid_in_slot(++cur_slot1_num);
        } while (cur_slot1_num < LK_HA_PG_SLOT_NUM && cur_level2_pgid == INVALID_PAGE_ID);
        
        cur_slot2_num = -1; // reset the second level page's slot num
    } while (cur_slot1_num < LK_HA_PG_SLOT_NUM); // loop in the first level page

    level1_page->r_unlock();
    bpm_->unpin_page(first_page_id_, false);
    tuple_->set_rid(RID(INVALID_PAGE_ID, INVALID_SLOT_NUM));
    slot1_num_ = INVALID_SLOT_NUM;
    slot2_num_ = INVALID_SLOT_NUM;
    
    return *this;
}

} // namespace dawn
