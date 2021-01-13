#pragma once

#include "storage/disk/disk_manager.h"
#include "meta/catalog.h"
#include "util/config.h"
#include "util/rwlatch.h"

namespace dawn {

class DBManager {
public:
    explicit DBManager(const string_t &meta_name, bool from_scratch = false) : status(false) {
        disk_manager_ = DiskManagerFactory::create_DiskManager(meta_name, from_scratch);
        if (disk_manager_ == nullptr)
            return;
        
        status = true;
    }

    page_id_t get_catalog_page_id() {
        
    }

private:
    DiskManager *disk_manager_;
    Catalog *catalog_;
    bool status;
    ReaderWriterLatch latch_;
};

} // namespace dawn
