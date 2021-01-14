#include "util/config.h"
#include "gtest/gtest.h"
#include "meta/catalog.h"
#include "meta/catalog_table.h"
#include "manager/db_manager.h"
#include "table/table_schema.h"
#include "meta/table_meta_data.h"
#include "table/column.h"
#include "data/types.h"

namespace dawn {

const char *meta = "test";
const char *meta_name = "test.mtd";
const char *db_name = "test.db";
const char *log_name = "test.log";

TableSchema* create_table_schema(std::vector<TypeId> types, std::vector<string_t> names, std::vector<size_t_> name_len) {
    if (types.size() != names.size())
        return nullptr;
    
    offset_t offset = 0;
    std::vector<Column> cols;
    size_t j = 0;
    for (size_t i = 0; i < types.size(); i++) {
        if (types[i] == TypeId::BOOLEAN) {
            cols.push_back(Column(names[i], offset, name_len[j]));
            offset += name_len[j++];
            continue;
        }
        cols.push_back(Column(types[i], names[i], offset));
        offset += Type::get_type_size(types[i]);
    }

    return new TableSchema(cols);
}

/**
 * Test List:
 *   1. ensure some TableMetaData can be created by catalog table and stored on the disk
 *   2. construct the pre-created TableMetaData by catalog table and ensure they are not changed
 *   3. delete some TableMetaData and ensure this operation can affect the disk
 *   
 */
TEST(CatalogTableTest, BasicTest) {
    
    {
        // test 1
        DBManager db_mgr(meta, true);
        ASSERT_TRUE(db_mgr.get_status());

        Catalog *catalog = db_mgr.get_catalog();
        ASSERT_NE(nullptr, catalog);

        CatalogTable *catalog_table = catalog->get_catalog_table();
        ASSERT_NE(nullptr, catalog_table);


    }


    remove(meta_name);
    remove(db_name);
    remove(log_name);
}

} // namespace dawn
