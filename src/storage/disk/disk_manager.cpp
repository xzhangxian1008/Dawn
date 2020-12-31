#include "storage/disk/disk_manager.h"

namespace dawn {

DiskManager::DiskManager(const string_t &meta_name, bool create) : status_(false) {
    if (create) {
        from_scratch(meta_name);
    } else {
        from_mtd(meta_name);
    }

    // db_io_.open(db_name_, std::ios::in);
    // if ((create && db_io_.is_open()) || (!create && !db_io_.is_open())) {
    //     // FIXME may be we can handle this in other ways
    //     if (create)
    //         PRINT("CREATE DB FILE FAIL:", db_name_, "exists");
    //     else
    //         PRINT("READ DB FILE FAIL:", "can not find", db_name_);
    //     db_io_.close();
    //     return;
    // }

    // log_io_.open(log_name_, std::ios::in);
    // if ((create && log_io_.is_open()) || (!create && !log_io_.is_open())) {
    //     // FIXME may be we can handle this in other ways
    //     if (create)
    //         PRINT("CREATE LOG FILE FAIL:", log_name_, "exists");
    //     else
    //         PRINT("READ LOG FILE FAIL:", "can not find", db_name_);
    //     log_io_.close();
    //     return;
    // }

    // // no more need to be done, if the mode is open
    // if (!create) {
    //     db_io_.seekp(0, std::ios::end);
    //     long pos = db_io_.tellp();
    //     next_page_id = (pos - PAGE_SIZE) / PAGE_SIZE + 1;
    //     status_ = true;
    //     return;
    // }
    
    // db_io_.clear();
    // log_io_.clear();

    // db_io_.open(db_name_, std::ios::out);
    // if (!db_io_.is_open()) {
    //     // FIXME may be we can handle this in other ways
    //     PRINT("CREATE DB FILE FAIL!");
    //     return;
    // }

    // log_io_.open(log_name_, std::ios::out);
    // if (!log_io_.is_open()) {
    //     // FIXME may be we can handle this in other ways
    //     PRINT("CREATE LOG FILE FAIL!");
    //     return;
    // }

    // next_page_id = 0;
    // status_ = true;
}

void DiskManager::from_scratch(const string_t &meta_name) {
    meta_name_ = meta_name + ".mtd";
    db_name_ = meta_name + ".db";
    log_name_ = meta_name + ".log";
    
    // ensure the files are inexistent
    if (!check_inexistence(meta_name_)) {
        string_t info("ERROR! ");
        info += meta_name_ + " exists";
        LOG(info);
        return;
    }

    if (!check_inexistence(db_name_)) {
        string_t info("ERROR! ");
        info += db_name_ + " exists";
        LOG(info);
        return;
    }

    if (!check_inexistence(log_name_)) {
        string_t info("ERROR! ");
        info += log_name_ + " exists";
        LOG(info);
        return;
    }

    // create corresponding files
    std::ios_base::openmode om = std::ios::in | std::ios::out | std::ios::trunc;

    if (!open_file(meta_name_, meta_io_, om)) {
        string_t info("ERROR! Open ");
        info += meta_name_ + " fail";
        LOG(info);
        return;
    }

    if (!open_file(db_name_, db_io_, om)) {
        string_t info("ERROR! Open ");
        info += db_name_ + " fail";
        char *rm_file = string2char(meta_name_);
        remove(rm_file);
        delete rm_file;
        LOG(info);
        return;
    }

    if (!open_file(log_name_, log_io_, om)) {
        string_t info("ERROR! Open ");
        info += log_name_ + " fail";
        char *rm_file = string2char(meta_name_);
        remove(rm_file);
        delete rm_file;
        rm_file = string2char(db_name_);
        remove(rm_file);
        delete rm_file;
        return;
    }

    // initialize the meta data
    max_ava_pgid_ = 100; // 0~99
    max_alloced_pgid_ = -1;
    for (int i = 0; i < 100; i++)
        free_pgid_.insert(i);
    
    // initialize the buffer with size 1024 byte
    meta_buffer = new char[512];
    buffer_size = 512;

    // write meta data to the buffer and set the offset
    int *p;
    
    db_name_sz_offset = 0;
    p = reinterpret_cast<int*>(meta_buffer + db_name_sz_offset);
    *p = db_name_.length();
    
    db_name_offset = db_name_sz_offset + sizeof(int);
    for (int i = 0; i < db_name_.length(); i++)
        meta_buffer[db_name_offset+i] = db_name_[i];
    db_name_[db_name_offset+db_name_.length()] = '\0';

    log_name_sz_offset = db_name_offset + db_name_.length() + 1;
    p = reinterpret_cast<int*>(meta_buffer+log_name_sz_offset);
    *p = log_name_.length();

    log_name_offset = log_name_sz_offset + sizeof(int);
    for (int i = 0; i < log_name_.length(); i++)
        meta_buffer[log_name_offset+i] = log_name_[i];
    meta_buffer[log_name_offset+log_name_.length()] = '\0';

    max_ava_pgid_offset = log_name_offset + log_name_.length() + 1;
    page_id_t *pt;
    pt = reinterpret_cast<page_id_t*>(meta_buffer+max_ava_pgid_offset);
    *pt = max_ava_pgid_;

    max_alloced_pgid_offset = max_ava_pgid_offset + sizeof(page_id_t);
    pt = reinterpret_cast<page_id_t*>(meta_buffer+max_alloced_pgid_offset);
    *pt = max_alloced_pgid_;

    reserved_offset = max_alloced_pgid_offset + sizeof(page_id_t);
    memset(meta_buffer + reserved_offset, 0, 128);

    // write meta data to the meta file
    meta_io_.seekg(0);
    meta_io_.write(meta_buffer, buffer_size);
    if (meta_io_.fail()) {
        // FIXME need other ways to handle the exception
        LOG("Write to meta data file fail.");
    }

    meta_io_.flush();
    status_ = true;
}

void DiskManager::from_mtd(const string_t &meta_name) {
    // TODO
}

/**
 * Ensure that we write data with the size of PAGE_SIZE
 */
bool DiskManager::write_page(page_id_t page_id, const char *data) {
    // if (page_id >= next_page_id) {
    //     return false;
    // }

    // if (!db_io_.is_open()) {
    //     string_t info("WRITE ERROR: can't open the ");
    //     info += db_name_;
    //     status_ = false;
    //     LOG(info);
    //     return false;
    // }

    // long offset = static_cast<long>(page_id) * PAGE_SIZE;
    // db_io_.seekg(offset);
    // db_io_.write(data, PAGE_SIZE);
    // if (db_io_.fail()) {
    //     LOG("WRITE FAIL!!!");
    //     return false;
    // }

    // db_io_.flush();
    return true;
}

/**
 * Each time, we can only read PAGE_SIZE
 */
bool DiskManager::read_page(page_id_t page_id, char *dst) {
    // if (page_id >= next_page_id) {
    //     return false;
    // }

    // if (!db_io_.is_open()) {
    //     string_t info("READ ERROR: can't open the ");
    //     info += db_name_;
    //     status_ = false;
    //     LOG(info);
    //     return false;
    // }

    // long offset = static_cast<long>(page_id) * PAGE_SIZE;
    // db_io_.seekp(offset);
    // db_io_.read(dst, PAGE_SIZE);
    // if (db_io_.gcount() != PAGE_SIZE) {
    //     string_t info("Get Error Data Size: ");
    //     info += std::to_string(db_io_.gcount());
    //     LOG(info);
    //     return false;
    // }
    
    return true;
}

page_id_t DiskManager::alloc_page() {
    // page_id_t new_page_id = -1;

    // if (!db_io_.is_open()) {
    //     string_t info("ALLOC ERROR: can't open the ");
    //     info += db_name_;
    //     status_ = false;
    //     LOG(info);
    //     return -1;
    // }

    // new_page_id = next_page_id++;
    // char c = 'c';
    // db_io_.seekg(((new_page_id+1)*PAGE_SIZE) - 1);
    // db_io_.write(&c, 1);
    // if (db_io_.fail()) {
    //     string_t info("ALLOC ERROR: can't alloc new space");
    //     LOG(info);
    //     next_page_id = -1;
    // }

    // return new_page_id;
}

bool DiskManager::write_meta_data() {

}

bool DiskManager::read_meta_data() {

}


} // namespace dawn