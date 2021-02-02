#pragma once

#include "table/tuple.h"
#include "table/table.h"

namespace dawn {

class TableIterator {
public:
    /**
     * Iterate from the designated position.
     */
    TableIterator(Table *table, RID rid) : table_(table) {
        tuple_ = new Tuple();
        if (rid.get_page_id() != INVALID_PAGE_ID) {
            if (!table->get_tuple(tuple_, rid))
                tuple_->set_rid(RID(INVALID_PAGE_ID, INVALID_SLOT_NUM));
        }
    }

    /**
     * Iterate from the Table's first tuple.
     */
    explicit TableIterator(Table *table) : table_(table) {
        tuple_ = new Tuple();
        table->get_the_first_tuple(tuple_);
    }

    TableIterator(const TableIterator &table_iter) : table_(table_iter.table_), tuple_(new Tuple(*(table_iter.tuple_))) {}

    ~TableIterator() { delete tuple_; }

    const Tuple& operator*() { return *tuple_; }

    Tuple* operator->() { return tuple_; }

    TableIterator &operator++();

    TableIterator operator++(int);

private:
    Table *table_;
    Tuple *tuple_;
};

} // namespace dawn
