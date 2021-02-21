#pragma once

#include <unordered_map>

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "storage/disk/disk_manager.h"
#include "buffer/buffer_pool_manager.h"
#include "meta/table_meta_data.h"
#include "table/table_schema.h"

namespace dawn {

/**
 * TODO store the table's key index and the index_header_page_id_ is useless
 * catalog table header layout:
 * ---------------------------------------------------------------------------
 * |                          common header (64)                          |
 * ---------------------------------------------------------------------------
 * | table_num (4) | tb_name offset 1 (4) | tb_name size 1 (4) | page id (4) |
 * ---------------------------------------------------------------------------
 * ---------------------------------------------------------------------------
 * |   ......   | tb_name offset n (4) | tb_name size n (4) | page id n (4) |
 * ---------------------------------------------------------------------------
 * ---------------------------------------------------------------------------
 * |  ...free space... | tb_name (x) | tb_name (x) |  ....  | tb_name (x) |
 * ---------------------------------------------------------------------------
 *                     ^
 *                     free space pointer
 * 
 * flush itself every time the data have been modified.
 * It's inefficient but can ensure the data stored on the disk.
 */
class CatalogTable {
public:
    explicit CatalogTable(page_id_t page_id, BufferPoolManager *bpm, bool from_scratch = false);

    DISALLOW_COPY(CatalogTable)

    ~CatalogTable() {
        // return the page
        bpm_->flush_page(self_page_id_);
        bpm_->unpin_page(self_page_id_, true);
        for (auto &p : tb_id_to_meta_)
            delete p.second;
    }

    table_id_t get_table_id(const string_t &tb_name) {
        latch_.r_lock();
        auto iter = tb_name_to_id_.find(tb_name);
        if (iter == tb_name_to_id_.end()) {
            latch_.r_unlock();
            return -1;
        }
        table_id_t table_id = iter->second;
        latch_.r_unlock();
        return table_id;
    }

    string_t get_table_name(table_id_t table_id) {
        latch_.r_lock();
        auto iter = tb_id_to_name_.find(table_id);
        if (iter == tb_id_to_name_.end()) {
            latch_.r_unlock();
            return string_t("");
        }
        string_t tb_name = iter->second;
        latch_.r_unlock();
        return tb_name;
    }
    
    inline int get_table_num() { return table_num_; }
    inline page_id_t get_page_id() const { return self_page_id_; }

    // ATTENTION no lock
    inline int get_free_space() const {
        return free_space_pointer_ - COM_PG_HEADER_SZ - 4 - tb_id_to_name_.size() * TABLE_RECORD_SZ;
    }

    TableMetaData* get_table_meta_data(string_t table_name) {
        latch_.r_lock();
        auto iter = tb_name_to_id_.find(table_name);
        latch_.r_unlock();
        if (iter == tb_name_to_id_.end()) {
            return nullptr;
        }
        return get_table_meta_data(iter->second);
    }

    TableMetaData* get_table_meta_data(table_id_t table_id);

    bool create_table(const string_t &table_name, const TableSchema &schema);
    bool delete_table(const string_t &table_name);
    bool delete_table(table_id_t table_id);
    std::vector<string_t> get_all_table_name();
    std::vector<table_id_t> get_all_table_id();
    string_t to_string();
    bool check_space(size_t_ size_needed){
        offset_t insert_offset = TABLE_NUM_OFFSET + table_num_ * TABLE_RECORD_SZ;
        size_t_ available_space = free_space_pointer_ - insert_offset;
        return available_space < size_needed ? false: true;
    }

    // TODO not support change table name so far
    bool change_table_name(table_id_t table_id, const string_t &new_name);
private:
    // ATTENTION no lock protects it
    void delete_table_data(table_id_t table_id, string_t table_name) {
        auto iter_id_to_meta = tb_id_to_meta_.find(table_id);
        if (iter_id_to_meta == tb_id_to_meta_.end()) {
            TableMetaData *tmd = new TableMetaData(bpm_, table_name, table_id);
            tb_id_to_meta_.insert(std::make_pair(table_id, tmd));
            iter_id_to_meta = tb_id_to_meta_.find(table_id);
        }
        iter_id_to_meta->second->delete_table_data();
        delete iter_id_to_meta->second;
        tb_id_to_meta_.erase(iter_id_to_meta);
        
        auto iter_id_to_name = tb_id_to_name_.find(table_id);
        tb_id_to_name_.erase(iter_id_to_name);

        auto iter_name_to_id = tb_name_to_id_.find(table_name);
        tb_name_to_id_.erase(iter_name_to_id);
    }

    // ATTENTION no lock
    string_t get_table_name(offset_t tb_name_offset, int size);
    TableMetaData* create_table_meta_data(table_id_t table_id);

    // call this function when catalog table has never been created before
    void init_catalog_table() {
        *reinterpret_cast<size_t_*>(data_ + TABLE_NUM_OFFSET) = 0;
    }

    static const int TABLE_RECORD_SZ = sizeof(offset_t) + sizeof(size_t_) + sizeof(page_id_t);
    static const offset_t TABLE_NUM_OFFSET = COM_PG_HEADER_SZ;

    BufferPoolManager *bpm_;
    page_id_t self_page_id_; // it's his own page id
    offset_t free_space_pointer_;

    // TableMetaData's page id is equal to table id
    std::unordered_map<table_id_t, string_t> tb_id_to_name_;
    std::unordered_map<string_t, table_id_t> tb_name_to_id_;
    std::atomic<int> table_num_;
    ReaderWriterLatch latch_;
    Page *page_;
    char *data_;

    // ATTENTION create-on-need, create TableMetaData only when we need it
    std::unordered_map<table_id_t, TableMetaData*> tb_id_to_meta_;
};

} // namespace dawn