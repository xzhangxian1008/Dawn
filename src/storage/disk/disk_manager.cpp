#include "storage/disk/disk_manager.h"

namespace dawn {

DiskManager::DiskManager(const string_t &meta_name, bool create) : status_(false) {
    meta_name_ = meta_name + ".mtd";
    db_name_ = meta_name + ".db";
    log_name_ = meta_name + ".log";
    
    if (create) {
        from_scratch();
    } else {
        from_mtd(meta_name);
    }
}

void DiskManager::from_scratch() {    
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
    
    // initialize the buffer with size 512 byte
    meta_buffer = new char[512];
    buffer_size = 512;

    catalog_page_id_ = get_new_page();

    if (write_meta_data()) {
        status_ = true;
    }
}

void DiskManager::from_mtd(const string_t &meta_name) {
    // Firstly, open the meta data file, read it and initialize the data
    if (!open_file(meta_name_, meta_io_, std::ios::in | std::ios::out)) {
        string_t info("ERROR! ");
        info += "Open " + meta_name_ + " Fail!";
        LOG(info);
        shutdown();
        return;
    }

    {
        // initialize the data with info in meta data file
        buffer_size = get_file_sz(meta_io_);
        if (buffer_size == -1) {
            string_t info("ERROR! ");
            info += "Get Invalid File Size!";
            LOG(info);
            shutdown();
            return;
        }
        meta_buffer = new char[buffer_size];

        meta_io_.seekg(0);
        meta_io_.read(meta_buffer, buffer_size);
        if (meta_io_.fail() || (meta_io_.gcount() != buffer_size)) {
            string_t info("ERROR! ");
            info += "Read " + meta_name_ + " Fail!";
            LOG(info);
            shutdown();
            return;
        }

        // get .db and .log file and open them
        int *p;
        db_name_sz_offset = 0;
        p = reinterpret_cast<int*>(meta_buffer + db_name_sz_offset); // get db_name size

        db_name_offset = db_name_sz_offset + sizeof(int);
        db_name_ = meta_buffer + db_name_offset;

        if (!open_file(db_name_, db_io_, std::ios::in | std::ios::out)) {
            string_t info("ERROR! ");
            info += "Open " + db_name_ + " Fail!";
            LOG(info);
            shutdown();
            return;
        }
        
        log_name_sz_offset = db_name_offset + *p + 1;
        p = reinterpret_cast<int*>(meta_buffer + log_name_sz_offset); // get log_name size

        log_name_offset = log_name_sz_offset + sizeof(int);
        log_name_ = meta_buffer + log_name_offset;

        if (!open_file(log_name_, log_io_, std::ios::in | std::ios::out)) {
            string_t info("ERROR! ");
            info += "Open " + log_name_ + " Fail!";
            LOG(info);
            shutdown();
            return;
        }
        
        // get some more data
        page_id_t *pg;
        max_ava_pgid_offset = log_name_offset + *p + 1;
        pg = reinterpret_cast<page_id_t*>(meta_buffer + max_ava_pgid_offset);
        max_ava_pgid_ = *pg;

        catalog_pgid_offset = max_ava_pgid_offset + PGID_T_SIZE;
        catalog_page_id_ = *reinterpret_cast<page_id_t*>(meta_buffer + catalog_pgid_offset);
    }

    // initialize the alloced_pgid_ and free_pgid_
    // Read through the .db file to check every page's status。
    // If one page is in use, we put it's page id into alloced_pgid
    // and update the max_alloced_pgid_ or put it into free_pgid.
    // When we are at the file's end, put rest of the page id into
    // free_pgid according to the max_ava_pgid_.

    long db_file_sz = get_file_sz(db_io_);

    if (db_file_sz == -1 || (db_file_sz % PAGE_SIZE != 0)) {
        string_t info("ERROR! ");
        info += "Get Invalid File Size! size:" + std::to_string(db_file_sz);
        LOG(info);
        shutdown();
        return;
    }

    if (db_file_sz == 0) {
        for (int i = 0; i < max_ava_pgid_; i++)
            free_pgid_.insert(i);
        max_alloced_pgid_ = -1;
        status_ = true;
        return;
    }

    char *tmp_buf = new char[READ_DB_BUF_SZ];
    long read_cnt = db_file_sz / READ_DB_BUF_SZ; // read the whole file need *read_cnt* IO times
    if (db_file_sz % READ_DB_BUF_SZ != 0)
        read_cnt++;

    /** 
     * read the full .db file to initialize data
     * for the efficiency of the initialization, we read a batch of pages each time
     */
    page_id_t page_id;
    int read_size;
    for (int i = 0; i < read_cnt; i++) {
        db_io_.seekp(i * READ_DB_BUF_SZ);

        read_size = READ_DB_BUF_SZ <= db_file_sz ? READ_DB_BUF_SZ : db_file_sz;
        db_io_.read(tmp_buf, read_size);
        if (db_io_.fail()) {
            string_t info("ERROR! Read Fail!");
            LOG(info);
            shutdown();
            return;
        }
        db_file_sz = db_file_sz - READ_DB_BUF_SZ;

        int read_sz = db_io_.gcount(); // read_sz refer to how large space we read
        char *p_status;
        for (int offset = 0; offset < read_sz; offset += PAGE_SIZE) {
            page_id = i * READ_DB_PG_NUM + (offset / PAGE_SIZE);

            p_status = reinterpret_cast<char*>(tmp_buf + offset);
            if (*p_status & STATUS_EXIST) {
                max_alloced_pgid_ = page_id;
                alloced_pgid_.insert(page_id);
            } else {
                free_pgid_.insert(page_id);
            }
        }
    }

    page_id++;
    while (page_id < max_ava_pgid_) {
        free_pgid_.insert(page_id++);
    }

    delete[] tmp_buf;
    status_ = true;
}

/**
 * Ensure that we write data with the size of PAGE_SIZE
 */
bool DiskManager::write_page(page_id_t page_id, const char *data) {
    if (!db_io_.is_open()) {
        string_t info("WRITE ERROR: can't open the ");
        info += db_name_;
        status_ = false;
        LOG(info);
        return false;
    }

    long offset = static_cast<long>(page_id) * PAGE_SIZE;
    db_io_latch_.w_lock();
    db_io_.seekg(offset);
    db_io_.write(data, PAGE_SIZE);
    if (db_io_.fail()) {
        db_io_latch_.w_unlock();
        LOG("WRITE FAIL!!!");
        return false;
    }
    db_io_latch_.w_unlock();

    db_io_.flush();
    return true;
}

/**
 * Each time, we can only read PAGE_SIZE
 */
bool DiskManager::read_page(page_id_t page_id, char *dst) {
    if (!db_io_.is_open()) {
        string_t info("READ ERROR: can't open the ");
        info += db_name_;
        status_ = false;
        LOG(info);
        return false;
    }

    long offset = static_cast<long>(page_id) * PAGE_SIZE;
    db_io_latch_.w_lock();
    db_io_.seekp(offset);
    db_io_.read(dst, PAGE_SIZE);
    if (db_io_.gcount() != PAGE_SIZE) {
        db_io_latch_.w_unlock();
        string_t info("Get Error Data Size: ");
        info += std::to_string(db_io_.gcount()) + ", page: " + std::to_string(page_id);
        LOG(info);
        return false;
    }
    db_io_latch_.w_unlock();
    
    return true;
}

page_id_t DiskManager::get_new_page() {
    if (!db_io_.is_open()) {
        string_t info("ALLOC ERROR: can't open the ");
        info += db_name_;
        status_ = false;
        LOG(info);
        return -1;
    }

    // get new page id, extract from free_pgid_ and insert into alloced_pgid_
    latch_.w_lock();
    page_id_t new_page_id;
    auto iter = free_pgid_.begin();
    if (iter == free_pgid_.end()) {
        // handle it, when there is no free page id
        int tmp = max_ava_pgid_;
        max_ava_pgid_ = max_ava_pgid_ * 2;
        for (int id = tmp; id < max_ava_pgid_; id++)
            free_pgid_.insert(id);
        iter = free_pgid_.begin();
    }
    new_page_id = *iter;
    free_pgid_.erase(iter);
    alloced_pgid_.insert(new_page_id);
    latch_.w_unlock();

    page_buf[0] = STATUS_EXIST;
    db_io_latch_.w_lock();
    db_io_.seekg(new_page_id * PAGE_SIZE);
    db_io_.write(page_buf, PAGE_SIZE);

    if (db_io_.fail()) {
        db_io_latch_.w_unlock();
        string_t info("ALLOC ERROR: can't alloc new space");
        LOG(info);
        latch_.w_lock();
        free_pgid_.insert(new_page_id);
        iter = alloced_pgid_.find(new_page_id);
        alloced_pgid_.erase(iter);
        new_page_id = -1;
        latch_.w_unlock();
    } else {
        if (max_alloced_pgid_ < new_page_id)
            max_alloced_pgid_ = new_page_id;
        db_io_.flush();
    }
    db_io_latch_.w_unlock();

    return new_page_id;
}

bool DiskManager::free_page(page_id_t page_id) {
    if (page_id < 0)
        return false;

    latch_.r_lock();
    auto iter = alloced_pgid_.find(page_id);
    if (iter == alloced_pgid_.end()) {
        latch_.r_unlock();
        return true;
    }
    latch_.r_unlock();

    // change the page's status on disk
    char c = STATUS_FREE;
    db_io_latch_.w_lock();
    db_io_.seekg(page_id * PAGE_SIZE);
    db_io_.write(&c, 1);
    if (db_io_.fail()) {
        db_io_latch_.w_unlock();
        LOG("free page fail");
        return false;
    }
    db_io_latch_.w_unlock();
    db_io_.flush();

    // update the meta data
    latch_.w_lock();
    free_pgid_.insert(page_id);
    alloced_pgid_.erase(iter);

    if (max_alloced_pgid_ == page_id) {
        if (alloced_pgid_.size() != 0) {
            iter = alloced_pgid_.end();
            iter--;
            max_alloced_pgid_ = *iter;
        } else {
            max_alloced_pgid_ = -1;
        }
    }
    latch_.w_unlock();

    return true;
}

bool DiskManager::write_meta_data() {
    if (meta_buffer == nullptr) {
        return false;
    }
    
    // write meta data to the buffer and set the offset
    int *p;
    
    db_name_sz_offset = 0;
    p = reinterpret_cast<int*>(meta_buffer + db_name_sz_offset);
    *p = db_name_.length();
    
    db_name_offset = db_name_sz_offset + OFFSET_T_SIZE;
    for (size_t i = 0; i < db_name_.length(); i++)
        meta_buffer[db_name_offset+i] = db_name_[i];
    meta_buffer[db_name_offset+db_name_.length()] = '\0';

    log_name_sz_offset = db_name_offset + db_name_.length() + 1;
    p = reinterpret_cast<int*>(meta_buffer+log_name_sz_offset);
    *p = log_name_.length();

    log_name_offset = log_name_sz_offset + OFFSET_T_SIZE;
    for (size_t i = 0; i < log_name_.length(); i++)
        meta_buffer[log_name_offset+i] = log_name_[i];
    meta_buffer[log_name_offset+log_name_.length()] = '\0';

    max_ava_pgid_offset = log_name_offset + log_name_.length() + 1;
    page_id_t *pt;
    pt = reinterpret_cast<page_id_t*>(meta_buffer+max_ava_pgid_offset);
    *pt = max_ava_pgid_;

    catalog_pgid_offset = max_ava_pgid_offset + PGID_T_SIZE;
    *reinterpret_cast<page_id_t*>(meta_buffer+catalog_pgid_offset) = catalog_page_id_;

    reserved_offset = catalog_pgid_offset + PGID_T_SIZE;
    memset(meta_buffer + reserved_offset, 0, 128);

    // write meta data to the meta file
    meta_io_.seekg(0);
    meta_io_.write(meta_buffer, buffer_size);
    if (meta_io_.fail()) {
        // FIXME need other ways to handle the exception
        LOG("Write to meta data file fail.");
        return false;
    }

    meta_io_.flush();
    return true;
}

} // namespace dawn