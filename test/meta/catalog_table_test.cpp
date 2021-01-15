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

/**
 * table name: table1
 * column names:
 * -----------------------
 * | tb1_col1 | tb1_col2 |
 * -----------------------
 * column types:
 * ------------------
 * | bool | integer |
 * ------------------
 */
string_t table1("table1");
std::vector<TypeId> tb1_types{TypeId::BOOLEAN, TypeId::INTEGER};
std::vector<string_t> tb1_names{"tb1_col1", "tb1_col2"};

/**
 * table name: table2
 * column names:
 * ----------------------------------
 * | tb2_col1 | tb2_col2 | tb2_col3 |
 * ----------------------------------
 * column types:
 * ----------------------------
 * | integer | bool | decimal |
 * ----------------------------
 */
string_t table2("table2");
std::vector<TypeId> tb2_types{TypeId::INTEGER, TypeId::BOOLEAN, TypeId::DECIMAL};
std::vector<string_t> tb2_names{"tb2_col1", "tb2_col2", "tb2_col3"};

/**
 * table name: table3
 * column names:
 * ---------------------------------------------
 * | tb3_col1 | tb3_col2 | tb3_col3 | tb3_col4 |
 * ---------------------------------------------
 * column types:
 * ----------------------------------------
 * | decimal | bool | char (10) | integer | 
 * ----------------------------------------
 */
string_t table3("table3");
std::vector<TypeId> tb3_types{TypeId::DECIMAL, TypeId::BOOLEAN, TypeId::CHAR, TypeId::INTEGER};
std::vector<string_t> tb3_names{"tb3_col1", "tb3_col2", "tb3_col3", "tb3_col4"};
std::vector<size_t_> tb3_char_size{10};

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

        TableSchema *tb1 = create_table_schema(tb1_types, tb1_names);
        TableSchema *tb2 = create_table_schema(tb2_types, tb2_names);
        TableSchema *tb3 = create_table_schema(tb3_types, tb3_names, tb3_char_size);

        EXPECT_TRUE(catalog_table->new_table(table1, *tb1));
        EXPECT_TRUE(catalog_table->new_table(table2, *tb2));
        EXPECT_TRUE(catalog_table->new_table(table3, *tb3));

        delete tb1;
        delete tb2;
        delete tb3;
    }

    remove(meta_name);
    remove(db_name);
    remove(log_name);
}

} // namespace dawn
