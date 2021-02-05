#pragma once

#include "storage/page/page.h"

namespace dawn {

/**
 * This Page is used for hash index's level 1 and level 2
 * LinkHashPage layout:
 * -----------------------------------------------------
 * |              common page header (64)              |
 * -----------------------------------------------------
 * -----------------------------------------------------
 * | refer page id 1 (4) | refer page id 2 (4) |  ...  |
 * -----------------------------------------------------
 */
class LinkHashPage : public Page {
public:
    void init() {
        for (offset_t i = 0; i < LK_HA_TOTAL_SLOT_NUM; i++)
            *reinterpret_cast<page_id_t*>(get_data() + FIRST_SLOT_NUM_OFFSET + i * SLOT_SIZE) = INVALID_PAGE_ID;
    }

    inline page_id_t get_pgid_in_slot(offset_t slot_num) {
        if (slot_num < 0 || slot_num >= LK_HA_TOTAL_SLOT_NUM) {
            return INVALID_PAGE_ID;
        }
        return *reinterpret_cast<page_id_t*>(get_data() + FIRST_SLOT_NUM_OFFSET + slot_num * SLOT_SIZE);
    }

    inline void set_pgid_in_slot(offset_t slot_num, page_id_t page_id) {
        if (slot_num < 0 || slot_num >= LK_HA_TOTAL_SLOT_NUM) return;
        *reinterpret_cast<page_id_t*>(get_data() + FIRST_SLOT_NUM_OFFSET + slot_num * SLOT_SIZE) = page_id;
    }

    /**
     * get the next page id from stat_slot(exclusive)
     * use INVALID_PAGE_ID if you want to start from the 0 slot
     * @return -1 shows that the start_slot is at the end
     */
    page_id_t get_next_pgid_in_slot(offset_t start_slot) {
        offset_t slot_num = start_slot == INVALID_PAGE_ID ? start_slot + 1 : 0;
        while (slot_num < LK_HA_TOTAL_SLOT_NUM) {
            page_id_t page_id = *reinterpret_cast<page_id_t*>(get_data() + FIRST_SLOT_NUM_OFFSET + slot_num * SLOT_SIZE) = page_id;
            if (page_id == INVALID_PAGE_ID) {
                slot_num++;
                continue;
            }
            return page_id;
        }
        return INVALID_PAGE_ID;
    }

private:
    static offset_t constexpr FIRST_SLOT_NUM_OFFSET = COM_PG_HEADER_SZ;
    static size_t_ constexpr SLOT_SIZE = PGID_T_SIZE;
};

} // namespace dawn
