#include "util/config.h"
#include "gtest/gtest.h"
#include "meta/catalog.h"
#include "manager/db_manager.h"

namespace dawn {

const char *meta = "test";
const char *meta_name = "test.mtd";
const char *db_name = "test.db";
const char *log_name = "test.log";

/**
 * Test List:
 *   1. ensure DBManager has created the Catalog from scratch
 *   2. ensure DBManager has created the pre-created Catalog
 */
TEST(CatalogTest, BasicTest) {
    page_id_t catalog_pgid;

    {
        // test 1
        DBManager db_mgr(meta, true);
        ASSERT_TRUE(db_mgr.get_status());
        EXPECT_NE(nullptr, db_mgr.get_catalog());
        catalog_pgid = db_mgr.get_catalog_page_id();

        // ensure the catalog table page id has been written to the disk
        DiskManager *disk_manager = db_mgr.get_disk_manager();
        char buf[PAGE_SIZE];
        disk_manager->read_page(catalog_pgid, buf);
        EXPECT_EQ(db_mgr.get_catalog()->get_catalog_table_page_id(),
            *reinterpret_cast<page_id_t*>(buf + Catalog::CATALOG_TABLE_PGID_OFFSET));
    }

    {
        // test 2
        DBManager db_mgr(meta);
        ASSERT_TRUE(db_mgr.get_status());
        EXPECT_NE(nullptr, db_mgr.get_catalog());
        EXPECT_EQ(catalog_pgid, db_mgr.get_catalog_page_id());
    }

    remove(meta_name);
    remove(db_name);
    remove(log_name);
}

} // namespace dawn
