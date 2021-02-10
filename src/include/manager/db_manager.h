#pragma once

#include <memory>

#include "storage/disk/disk_manager.h"
#include "buffer/buffer_pool_manager.h"
#include "meta/catalog.h"
#include "util/config.h"
#include "util/rwlatch.h"

namespace dawn {

class DBManager {
public:
    explicit DBManager(const string_t &meta_name, bool from_scratch = false) 
        : status(false) {
        disk_manager_ = DiskManagerFactory::create_DiskManager(meta_name, from_scratch);
        if (disk_manager_ == nullptr)
            return;
        
        bpm_ = new BufferPoolManager(disk_manager_, DEFAULT_POOL_SIZE);
        catalog_page_id_ = disk_manager_->get_catalog_pgid();
        catalog_ = new Catalog(bpm_, catalog_page_id_, from_scratch);
        status = true;
    }

    ~DBManager() {
        if (!bpm_->flush_all()) {
            LOG("Flush all fail");
        }
        delete catalog_;
        delete bpm_;
        delete disk_manager_;
    }

    inline page_id_t get_catalog_page_id() const { return catalog_page_id_; }
    inline DiskManager* get_disk_manager() const { return disk_manager_; }
    inline BufferPoolManager* get_buffer_pool_manager() const { return bpm_; }
    inline Catalog* get_catalog() const { return catalog_; }
    inline bool get_status() const { return status; }

    static inline void set_default_pool_size(size_t_ pool_size) {
        DEFAULT_POOL_SIZE = pool_size;
    }

    static inline size_t_ get_default_pool_size() {
        return DEFAULT_POOL_SIZE;
    }

private:
    static size_t_ DEFAULT_POOL_SIZE;

    DiskManager *disk_manager_;
    BufferPoolManager *bpm_;
    Catalog *catalog_;
    page_id_t catalog_page_id_;
    
    bool status;
    ReaderWriterLatch latch_;
};

} // namespace dawn
