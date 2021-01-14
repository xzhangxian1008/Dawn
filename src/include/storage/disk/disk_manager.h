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
 * | max_ava_pgid_ (4) |  Reserved (128)    |
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

    // TODO write, read and free for meta data file

    bool write_page(page_id_t page_id, const char *data);
    bool read_page(page_id_t page_id, char *dst);
    page_id_t get_new_page();
    bool free_page(page_id_t page_id);
    // bool write_log_page();
    // bool read_log_page();
    inline bool get_status() const { return status_; }
    inline page_id_t get_max_ava_pgid() const { return max_ava_pgid_; }
    inline page_id_t get_max_alloced_pgid() const { return max_alloced_pgid_; }
    inline page_id_t get_catalog_pgid() const { return catalog_page_id_; }

    // duplicated, just for test
    bool is_free(page_id_t page_id) const { return free_pgid_.find(page_id) != free_pgid_.end(); }
    bool is_allocated(page_id_t page_id) const { return alloced_pgid_.find(page_id) != alloced_pgid_.end();}

    void shutdown() {
        if (status_ && !write_meta_data()) {
            LOG("WARNING! Write Meta Data Fail in Shutdown");
        }
        status_ = false;
        meta_io_.close();
        db_io_.close();
        log_io_.close();
        if (meta_buffer != nullptr) {
            delete[] meta_buffer;
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
    offset_t catalog_pgid_offset;
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

    page_id_t catalog_page_id_;

    ReaderWriterLatch latch_;

    ReaderWriterLatch db_io_latch_;
    ReaderWriterLatch log_io_latch_;
};

class DiskManagerFactory {
public:
    static void add_meta_postfix(char *meta_name, offset_t offset) {
        meta_name[offset] = '.';
        meta_name[offset+1] = 'm';
        meta_name[offset+2] = 't';
        meta_name[offset+3] = 'd';
        meta_name[offset+4] = '\0';
    }

    static void add_db_postfix(char *db_name, offset_t offset) {
        db_name[offset] = '.';
        db_name[offset+1] = 'd';
        db_name[offset+2] = 'b';
        db_name[offset+3] = '\0';
    }

    static void add_log_postfix(char *log_name, offset_t offset) {
        log_name[offset] = '.';
        log_name[offset+1] = 'l';
        log_name[offset+2] = 'o';
        log_name[offset+3] = 'g';
        log_name[offset+4] = '\0';
    }

    static void copy_string2char(char *s, const string_t &str) {
        int size = str.length();
        for (int i = 0; i < size; i++)
            s[i] = str[i];
    }

    static DiskManager* create_DiskManager(const string_t &meta_name, bool create = false) {
        char meta_name_[meta_name.length() + 5];
        char db_name_[meta_name.length() + 4];
        char log_name_[meta_name.length() + 5];

        copy_string2char(meta_name_, meta_name);
        copy_string2char(db_name_, meta_name);
        copy_string2char(log_name_, meta_name);
        add_meta_postfix(meta_name_, meta_name.length());
        add_db_postfix(db_name_, meta_name.length());
        add_log_postfix(log_name_, meta_name.length());

        if (create) {
            remove(meta_name_);
            remove(db_name_);
            remove(log_name_);
        }

        DiskManager *dm = new DiskManager(meta_name, create);
        if ((dm->get_status() == false) || check_inexistence(meta_name_)
            || check_inexistence(db_name_) || check_inexistence(log_name_)) {
            delete dm;
            return nullptr;
        }

        return dm;
    }
};

} // namespace dawn