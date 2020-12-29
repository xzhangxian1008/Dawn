#include "storage/disk/disk_manager.h"

namespace dawn {

DiskManager::DiskManager(std::string &db_name, bool create) : status(false), next_page_id(-1) {
    db_name_ = db_name + ".db";
    log_name_ = db_name + ".log";

    db_io_.open(db_name_, std::ios::in);
    if ((create && db_io_.is_open()) || (!create && !db_io_.is_open())) {
        // FIXME may be we can handle this in other ways
        if (create)
            PRINT("CREATE DB FILE FAIL:", db_name_, "exists");
        else
            PRINT("READ DB FILE FAIL:", "can not find", db_name_);
        db_io_.close();
        return;
    }

    log_io_.open(log_name_, std::ios::in);
    if ((create && log_io_.is_open()) || (!create && !log_io_.is_open())) {
        // FIXME may be we can handle this in other ways
        if (create)
            PRINT("CREATE LOG FILE FAIL:", log_name_, "exists");
        else
            PRINT("READ LOG FILE FAIL:", "can not find", db_name_);
        log_io_.close();
        return;
    }

    // no more need to be done, if the mode is open
    if (!create) {
        db_io_.seekp(0, std::ios::end);
        long pos = db_io_.tellp();
        next_page_id = (pos - PAGE_SIZE) / PAGE_SIZE + 1;
        status = true;
        return;
    }
    
    db_io_.clear();
    log_io_.clear();

    db_io_.open(db_name_, std::ios::out);
    if (!db_io_.is_open()) {
        // FIXME may be we can handle this in other ways
        PRINT("CREATE DB FILE FAIL!");
        return;
    }

    log_io_.open(log_name_, std::ios::out);
    if (!log_io_.is_open()) {
        // FIXME may be we can handle this in other ways
        PRINT("CREATE LOG FILE FAIL!");
        return;
    }

    next_page_id = 0;
    status = true;
}

/**
 * Ensure that we write data with the size of PAGE_SIZE
 */
bool DiskManager::write_page(page_id_t page_id, const char *data) {
    if (page_id >= next_page_id) {
        return false;
    }

    long offset = static_cast<long>(page_id) * PAGE_SIZE;
    db_io_.seekg(offset);
    db_io_.write(data, PAGE_SIZE);
    if (db_io_.fail()) {
        LOG("WRITE FAIL!!!");
        return false;
    }

    db_io_.flush();
    return true;
}

/**
 * We can only read the PAGE_SIZE
 */
bool DiskManager::read_page(page_id_t page_id, char *dst) {
    if (page_id >= next_page_id) {
        return false;
    }

    long offset = static_cast<long>(page_id) * PAGE_SIZE;
    db_io_.seekp(offset);
    db_io_.read(dst, PAGE_SIZE);
    if (db_io_.gcount() != PAGE_SIZE) {
        std::string info("Get Error Data Size: ");
        info += std::to_string(db_io_.gcount());
        LOG(info);
        return false;
    }
    
    return true;
}

page_id_t DiskManager::alloc_page(char *dst) {
    // TODO write something into file to fill the space
}

void DiskManager::shutdown() {
    db_io_.close();
    log_io_.close();
}

} // namespace dawn