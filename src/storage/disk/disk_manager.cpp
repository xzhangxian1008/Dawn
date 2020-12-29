#include "storage/disk/disk_manager.h"

namespace dawn {

DiskManager::DiskManager(std::string &file_name, bool create) {
    file_name_ = file_name_ + ".db";
    log_name_ = file_name_ + ".log";

    file_io_.open(file_name_, std::ios::in);
    if (create && file_io_.is_open()) {
        // FIXME may be we can handle this in other ways
        PRINT("CREATE DB FILE FAIL:", file_name_, "exists");
        file_io_.close();
        return;
    }

    log_io_.open(log_name_, std::ios::in);
    if (create && log_io_.is_open()) {
        // FIXME may be we can handle this in other ways
        PRINT("CREATE LOG FILE FAIL:", log_name_, "exists");
        log_io_.close();
        return;
    }

    // no more need to be done, if the mode is open
    if (!create)
        return;
    
    file_io_.clear();
    log_io_.clear();

    file_io_.open(file_name_, std::ios::out);
    if (!file_io_.is_open()) {
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
}

void DiskManager::write_page(page_id_t page_id, const char *src) {

}

void DiskManager::read_page(page_id_t page_id, const char *dst) {

}

} // namespace dawn