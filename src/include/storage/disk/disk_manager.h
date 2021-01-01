#pragma once

#include <fstream>
#include <string>
#include <atomic>
#include <set>
#include <string.h>

#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"

namespace dawn {
/**
 * TODO handle concurrency environment
 * 
 * meta data file layout:
 * the reserved filed is placed just for notification not to forget to
 * maintain a reserved field when I plan to add other fields.
 * 
 * we can add a field for the check purpose to ensure it's a valid
 * .mtd file, but now, it's unnecessary.
 * 
 * --------------------------------------------------------------------
 * | db_name size (4) | db_name... | log_name size (4) | log_name ... |
 * --------------------------------------------------------------------
 * --------------------------------------------------------------------
 * | max_ava_pgid_ (4) | max_alloced_pgid_ (4) |    Reserved (128)    |
 * --------------------------------------------------------------------
 */
class DiskManager {
friend class DiskManager_T;
public:
    /**
     * 
     * @param meta_name the file name of the database file to write to
     * @param create open or create the file? [true: create] [false: open]
     */
    explicit DiskManager(const string_t &meta_name, bool create = false);

    ~DiskManager() {
        if (status_)
            shutdown();
    }

    bool write_page(page_id_t page_id, const char *data);
    bool read_page(page_id_t page_id, char *dst);
    page_id_t get_new_page();
    bool free_page(page_id_t page_id);
    // bool write_log();
    // bool read_log();
    bool get_status() const { return status_; }
    page_id_t get_max_ava_pgid() const { return max_ava_pgid_; }
    page_id_t get_max_alloced_pgid() const { return max_alloced_pgid_; }

    // duplicated, just for test
    bool is_free(page_id_t page_id) const { return free_pgid_.find(page_id) != free_pgid_.end(); }
    bool is_allocated(page_id_t page_id) const { return alloced_pgid_.find(page_id) != alloced_pgid_.end();}

    void shutdown() {
        // FIXME bug will happen if we put log_io_.close() after the write_meta_data(). confused...
        if (status_ && !write_meta_data()) {
            LOG("WARNING! Write Meta Data Fail in Shutdown");
        }
        status_ = false;
        meta_io_.close();
        db_io_.close();
        log_io_.close();
        if (meta_buffer != nullptr) {
            delete meta_buffer;
            buffer_size = -1;
            meta_buffer = nullptr;
        }
    }

private:
    void from_scratch();
    void from_mtd(const string_t &meta_name);
    bool write_meta_data();

    offset_t db_name_sz_offset;
    offset_t db_name_offset;
    offset_t log_name_sz_offset;
    offset_t log_name_offset;
    offset_t max_ava_pgid_offset;
    offset_t max_alloced_pgid_offset;
    offset_t reserved_offset;

    fstream_t db_io_;
    string_t db_name_;
    fstream_t log_io_;
    string_t log_name_;
    fstream_t meta_io_;
    string_t meta_name_;

    /**
     * check if manager works normally.
     * Only when it works normally, can we trust the data in it.
     *   true: yes
     *   false: no
     */
    bool status_;

    /** 
     * all available page ids are lower or equal to the max_ava_pgid.
     * if they are exhausted, we should multiply it by 2.
     * eg. max_ava_pgid_ == 100 ==> page id:0~99
     */
    std::atomic<page_id_t> max_ava_pgid_;

    /**
     * TODO truncate the .db file
     * max page id that has been allocated, so that we can
     * truncate the file to shrink it's size.
     */
    std::atomic<page_id_t> max_alloced_pgid_;

    /** 
     * store the page id that have been allocated
     * scan the .db file every start time to find all the allocated pages.
     * Stupid way, just for simple.
     */
    std::set<page_id_t> alloced_pgid_;

    // store the free page id, we should get the smallest page id each time
    std::set<page_id_t> free_pgid_;

    // a buffer to store the meta data content on the disk
    char *meta_buffer = nullptr;

    // when buffer is insufficient, multiply it by 2.
    int buffer_size = -1;

    char page_buf[PAGE_SIZE];

    ReaderWriterLatch latch_;

    // TODO ensure the io concurrency
    ReaderWriterLatch io_latch_;
};
} // namespace dawn