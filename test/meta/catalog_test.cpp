#include "util/config.h"
#include "gtest/gtest.h"
#include "meta/catalog.h"
#include "manager/db_manager.h"

namespace dawn {

/**
 * Test List:
 *   1. ensure DBManager has created the Catalog from scratch
 *   2. ensure DBManager has created the pre-created Catalog
 *   2. create Catalog from scratch and ensure the disk has store the catalog_table's page id
 *   3. create a pre-created Catalog and ensure get the catalog_table's page id stored in the disk
 */
TEST(CatalogTest, BasicTest) {
    {
        // test 1

    }

    {
        // test 2
    }

    {
        // test 3
    }

    {
        // test 4
    }
}

} // namespace dawn
