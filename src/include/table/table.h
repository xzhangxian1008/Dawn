#pragma once

#include "buffer/buffer_pool_manager.h"
#include "storage/page/page.h"
#include "storage/page/table_page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/rid.h"
#include "table/tuple.h"
#include "index/link_hash.h"
#include "mutex"

namespace dawn {

/**
 * ATTENTION problem
 * delete、update and get tuples could be done with the RID parameter directly.
 * Is it safe? How can I ensure they are legal RID and won't do some strange operation?
 * Do we need to maintain another data structure to ensure the RID are legal?
 */
class Table {
    friend class TableIterator;
public:
    // if from_scratch == true, it means that the Table should initialize the page in the disk
    Table(BufferPoolManager *bpm, const page_id_t first_table_page_id, bool from_scratch = false);
    ~Table() = default;
    void delete_all_data();
    page_id_t get_first_table_page_id() const { return first_table_page_id_; }
    // bool get_the_first_tuple(Tuple *tuple) const;

    bool insert_tuple(Tuple *tuple, const Schema &tb_schema);
    bool mark_delete(const Value &key_value, const Schema &tb_schema);
    bool mark_delete(const RID &rid);
    void apply_delete(const Value &key_value, const Schema &tb_schema);
    void apply_delete(const RID &rid);
    void rollback_delete(const Value &key_value, const Schema &tb_schema);
    void rollback_delete(const RID &rid);

    /**
     * @param new_tuple new position will be set in the new_tuple
     */
    bool update_tuple(Tuple *new_tuple, const RID &old_rid, const Schema &tb_schema);

    /**
     * search tuple with index, the rid will be set in the parameter *tuple
     */
    bool get_tuple(const Value &key_value, Tuple *tuple, const Schema &tb_schema);

    /** get tuple directly */
    bool get_tuple(Tuple *tuple, const RID &rid);

    index_code_t get_index_type() const { return index_type_; }
private:
    BufferPoolManager *bpm_;
    const page_id_t first_table_page_id_; // TODO initialize it at first
    ReaderWriterLatch latch_;
    index_code_t index_type_ = LINK_HASH;

    op_code_t (*insert_tuple_func)(INSERT_TUPLE_FUNC_PARAMS);
    op_code_t (*mark_delete_func)(MARK_DELETE_FUNC_PARAMS);
    void (*apply_delete_func)(APPLY_DELETE_FUNC_PARAMS);
    void (*rollback_delete_func)(ROLLBACK_DELETE_FUNC_PARAMS);
    op_code_t (*get_tuple_func)(GET_TUPLE_FUNC_PARAMS);
    op_code_t (*update_tuple_func)(UPDATE_TUPLE_FUNC_PARAMS);
};

} // namespace dawn
