#include "table/table_iterator.h"

namespace dawn {

TableIterator& TableIterator::operator++() {
    TablePage *table_page = reinterpret_cast<TablePage*>(table_->bpm_->get_page(tuple_->get_rid().get_page_id()));
    RID rid;

    table_page->r_lock();
    while (!table_page->get_next_tuple_rid(tuple_->get_rid(), &rid)) {
        page_id_t next_page_id = table_page->get_next_page_id();
        table_page->r_unlock();
        table_->bpm_->unpin_page(table_page->get_page_id(), false);

        if (next_page_id == INVALID_PAGE_ID) {
            tuple_->set_rid(RID(INVALID_PAGE_ID, INVALID_SLOT_NUM));
            return *this;
        }

        // jump to the next TablePage
        table_page = reinterpret_cast<TablePage*>(table_->bpm_->get_page(next_page_id));
        table_page->r_lock();
    }
    table_page->get_tuple(tuple_, rid);
    table_page->r_unlock();
    table_->bpm_->unpin_page(table_page->get_page_id(), false);
    return *this;
}

TableIterator TableIterator::operator++(int) {
    TableIterator clone(*this);
    ++(*this);
    return clone;
}

} // namespace dawn
