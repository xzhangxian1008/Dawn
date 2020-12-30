#pragma once

#include <fstream>
#include <string>
#include <atomic>
#include <set>

#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"

namespace dawn {
/**
 * meta data file layout:
 * --------------------------------------------------------------------
 * | db_name size (4) | db_name... | log_name size (4) | log_name ... |
 * --------------------------------------------------------------------
 * --------------------------------------------------------------------
 * | max_ava_pgid_ (4) | max_alloced_pgid_ (4) | alloced_pgid_ num (4)|
 * --------------------------------------------------------------------
 * --------------------------------------------------------------------
 * | alloced_pgid_ 1 (4) | alloced_pgid_ 2 | ... |   alloced_pgid_ n  |
 * --------------------------------------------------------------------
 */
class DiskManager {
public:
    /**
     * 
     * @param meta_name the file name of the database file to write to
     * @param create open or create the file? [true: create] [false: open]
     */
    explicit DiskManager(const string_t &meta_name, bool create = false);

    ~DiskManager() {
        meta_io_.close();
        db_io_.close();
        log_io_.close();
        if (meta_buffer != nullptr) {
            delete meta_buffer;
        }
    }

    bool write_page(page_id_t page_id, const char *data);
    bool read_page(page_id_t page_id, char *dst);
    page_id_t alloc_page();
    bool free_page(page_id_t page_id);
    // bool write_log();
    // bool read_log();
    bool get_status() const { return status_; }

private:
    void from_scratch(const string_t &meta_name);
    void from_mtd(const string_t &meta_name);

    /**
     * ensure the file is inexistent
     * @return true: not exist  false: exist
     */
    bool check_inexistence(const string_t &file_name);

    /**
     * open file with given open mode
     * @return true: successful  false: unsuccessful
     */
    bool open_file(const string_t &file_name, std::fstream &io, std::ios_base::openmode &om);

    bool write_meta_data();
    bool read_meta_data();

    offset_t db_name_sz_offset = 0;
    offset_t log_name_sz_offset;
    offset_t max_ava_pgid_sz_offset;
    offset_t max_alloced_pgid_sz_offset;
    offset_t alloced_pgid_num_offset;

    std::fstream db_io_;
    string_t db_name_;
    std::fstream log_io_;
    string_t log_name_;
    std::fstream meta_io_;
    string_t meta_name_;

    /**
     * check if manager works normally.
     *   true: yes
     *   false: no
     */
    bool status_;

    /** 
     * all available page ids are lower or equal to the max_ava_pgid
     * if they are exhausted, we should multiply it by 2.
     * eg. max_ava_pgid_ == 100 ==> page id:0~99
     */
    std::atomic<page_id_t> max_ava_pgid_;

    /**
     * TODO truncating hasn't been implemented
     * max page id that has been allocated, so that we can
     * truncate the file to shrink it's size.
     */
    std::atomic<page_id_t> max_alloced_pgid_;

    /**
     * store the page id that have been allocated
     */
    std::set<page_id_t> alloced_pgid_;

    /**
     * store the free page id, we should get the smallest page id each time
     */
    std::set<page_id_t> free_pgid_;

    /**
     * a buffer to store the meta data content on the disk
     */
    char *meta_buffer = nullptr;

    int buffer_size = -1;

    ReaderWriterLatch latch_;
};
} // namespace dawn