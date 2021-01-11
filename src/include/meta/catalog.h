#pragma once

#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "buffer/buffer_pool_manager.h"
#include "meta/catalog_table.h"

namespace dawn {


/**
 * catalog layout：
 * ------------------------------------------------------------------------
 * |                          common header (64)                          |
 * ------------------------------------------------------------------------
 * | catalog table page id (4) |
 * ------------------------------------------------------------------------
 */
class Catalog {
public:
    Catalog(BufferPoolManager *bpm, const page_id_t self_page_id, bool from_scratch = false)
        : bpm_(bpm), self_page_id_(self_page_id) {
        page_ = bpm->get_page(self_page_id_);
        if (page_ == nullptr) {
            PRINT("Catalog: ERROR! get nullptr, page id ", self_page_id_);
            exit(-1);
        }
        data_ = page_->get_data();

        if (from_scratch)
            init_catalog();

        // initialize CatalogTable
        page_id_t catalog_table_page_id = 
            *reinterpret_cast<page_id_t*>(data_ + CATALOG_TABLE_PGID_OFFSET);
        catalog_table_ = new CatalogTable(catalog_table_page_id, bpm_, from_scratch);
    }

    ~Catalog() {
        delete catalog_table_;
    }

    // TODO operation about catalog table
    
private:
    // call this function when the catalog has never been created before
    inline void init_catalog() {
        Page *p = bpm_->new_page();
        if (p == nullptr) {
            PRINT("Catalog.init_catalog: ERROR! get nullptr, page id ", self_page_id_);
            exit(-1);
        }

        *reinterpret_cast<page_id_t*>(data_ + CATALOG_TABLE_PGID_OFFSET) = p->get_page_id();
        bpm_->unpin_page(p->get_page_id(), true);
    }

    static const offset_t CATALOG_TABLE_PGID_OFFSET = COM_PG_HEADER_SZ;

    BufferPoolManager *bpm_;
    const page_id_t self_page_id_;
    CatalogTable *catalog_table_;

    Page *page_;
    char *data_;
};

} // namespace dawn
