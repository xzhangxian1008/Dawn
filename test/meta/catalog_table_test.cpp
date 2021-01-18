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
 * table name: table0
 * column names:
 * ------------
 * | tb1_col1 |
 * ------------
 * column types:
 * --------
 * | bool |
 * --------
 */
string_t table0("table0");
std::vector<TypeId> tb0_col_types{TypeId::BOOLEAN};
std::vector<string_t> tb0_col_names{"tb0_col1"};
std::vector<size_t_> tb0_char_size;
size_t_ tb0_tuple_size = Type::get_bool_size();

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
std::vector<TypeId> tb1_col_types{TypeId::BOOLEAN, TypeId::INTEGER};
std::vector<string_t> tb1_col_names{"tb1_col1", "tb1_col2"};
std::vector<size_t_> tb1_char_size;
size_t_ tb1_tuple_size = Type::get_bool_size() + Type::get_integer_size();

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
std::vector<TypeId> tb2_col_types{TypeId::INTEGER, TypeId::BOOLEAN, TypeId::DECIMAL};
std::vector<string_t> tb2_col_names{"tb2_col1", "tb2_col2", "tb2_col3"};
std::vector<size_t_> tb2_char_size;
size_t_ tb2_tuple_size = Type::get_integer_size() + Type::get_bool_size() + Type::get_decimal_size();

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
std::vector<TypeId> tb3_col_types{TypeId::DECIMAL, TypeId::BOOLEAN, TypeId::CHAR, TypeId::INTEGER};
std::vector<string_t> tb3_col_names{"tb3_col1", "tb3_col2", "tb3_col3", "tb3_col4"};
size_t_ tb3_char0_sz = 10;
std::vector<size_t_> tb3_char_size{tb3_char0_sz};
size_t_ tb3_tuple_size = Type::get_decimal_size() + Type::get_bool_size() + tb3_char0_sz + Type::get_integer_size();

/**
 * table name: table4
 * column names:
 * --------------------------------------------------------
 * | tb4_col1 | tb4_col2 | tb4_col3 | tb4_col4 | tb4_col5 |
 * --------------------------------------------------------
 * column types:
 * ----------------------------------------------------
 * | integer | char (20) | bool | char (10) | decimal | 
 * ----------------------------------------------------
 */
string_t table4("table4");
std::vector<TypeId> tb4_col_types{TypeId::INTEGER, TypeId::CHAR, TypeId::BOOLEAN, TypeId::CHAR, TypeId::DECIMAL};
std::vector<string_t> tb4_col_names{"tb4_col1", "tb4_col2", "tb4_col3", "tb4_col4", "tb4_col5"};
size_t_ tb4_char0_sz = 20;
size_t_ tb4_char1_sz = 10;
std::vector<size_t_> tb4_char_size{20, 10};
size_t_ tb4_tuple_size = Type::get_integer_size() + tb4_char0_sz + Type::get_bool_size() + tb4_char1_sz + Type::get_decimal_size();

/**
 * Test List:
 *   1. ensure some TableMetaData can be created by catalog table
 *   2. construct the pre-created TableMetaData by catalog table and ensure they are not changed
 *   3. delete some TableMetaData and ensure this operation can affect the disk
 */
TEST(CatalogTableTest, BasicTest) {
    std::vector<string_t> tables{table0, table1, table2, table3, table4};
    std::vector<std::vector<TypeId>> tb_col_types{tb0_col_types, tb1_col_types, tb2_col_types, tb3_col_types, tb4_col_types};
    std::vector<std::vector<string_t>> tb_col_names{tb0_col_names, tb1_col_names, tb2_col_names, tb3_col_names, tb4_col_names};
    std::vector<std::vector<size_t_>> tb_char_size{tb0_char_size, tb1_char_size, tb2_char_size, tb3_char_size, tb4_char_size};
    std::vector<size_t_> tb_tuple_size{tb0_tuple_size, tb1_tuple_size, tb2_tuple_size, tb3_tuple_size, tb4_tuple_size};

    size_t_ tb_num = tables.size();
    std::vector<table_id_t> table_id(tb_num);
    std::vector<TableSchema*> table_schema(tb_num);

    {
        // test 1
        DBManager db_mgr(meta, true);
        ASSERT_TRUE(db_mgr.get_status());

        Catalog *catalog = db_mgr.get_catalog();
        ASSERT_NE(nullptr, catalog);

        CatalogTable *catalog_table = catalog->get_catalog_table();
        ASSERT_NE(nullptr, catalog_table);

        for (int i = 0; i < tb_num; i++) {
            table_schema[i] = create_table_schema(tb_col_types[i], tb_col_names[i], tb_char_size[i]);
            EXPECT_NE(table_schema[i], nullptr);
            EXPECT_EQ(table_schema[i]->get_tuple_size(), tb_tuple_size[i]);
            EXPECT_TRUE(catalog_table->new_table(tables[i], *(table_schema[i])));
            table_id[i] = catalog_table->get_table_id(tables[i]);
        }
    }

    {
        // test 2
        DBManager db_mgr(meta);
        ASSERT_TRUE(db_mgr.get_status());

        Catalog *catalog = db_mgr.get_catalog();
        ASSERT_NE(nullptr, catalog);

        CatalogTable *catalog_table = catalog->get_catalog_table();
        ASSERT_NE(nullptr, catalog_table);

        EXPECT_EQ(tb_num, catalog_table->get_table_num());

        std::vector<TableMetaData*> tmd(tb_num);
        std::vector<const TableSchema*> ts(tb_num);
        for (int i = 0; i < tb_num; i++) {
            EXPECT_EQ(catalog_table->get_table_name(table_id[i]), tables[i]);
            EXPECT_EQ(catalog_table->get_table_id(tables[i]), table_id[i]);
            tmd[i] = catalog_table->get_table_meta_data(table_id[i]);
            EXPECT_NE(tmd[i], nullptr);
            ts[i] = tmd[i]->get_table_schema();
            EXPECT_TRUE(is_table_schemas_equal(*(ts[i]), *(table_schema[i])));
        }
    }

    {
        // test 3
        std::set<int> deleted_idx{1, 3}; // refer to the index of the deleted table
        std::set<table_id_t> deleted_tb_id;
        std::set<string_t> deleted_tb_name;

        int before_delete_free_space;
        int after_delete_free_space;

        for (auto idx : deleted_idx) {
            deleted_tb_id.insert(table_id[idx]);
            deleted_tb_name.insert(tables[idx]);
        }

        {
            // delete the table and check
            DBManager db_mgr(meta);
            ASSERT_TRUE(db_mgr.get_status());

            Catalog *catalog = db_mgr.get_catalog();
            ASSERT_NE(nullptr, catalog);

            CatalogTable *catalog_table = catalog->get_catalog_table();
            ASSERT_NE(nullptr, catalog_table);

            EXPECT_EQ(tb_num, catalog_table->get_table_num());

            before_delete_free_space = catalog_table->get_free_space();

            // do delete operation
            for (auto i : deleted_tb_id)
                EXPECT_TRUE(catalog_table->delete_table(i));
            
            // check
            EXPECT_EQ(tb_num - deleted_idx.size(), catalog_table->get_table_num());

            after_delete_free_space = catalog_table->get_free_space();
            EXPECT_LT(before_delete_free_space, after_delete_free_space); // simple check

            for (auto i : deleted_tb_id)
                EXPECT_EQ(string_t(""), catalog_table->get_table_name(i));
            
            for (auto &name : deleted_tb_name)
                EXPECT_EQ(-1, catalog_table->get_table_id(name));
            
            BufferPoolManager *bpm = catalog->get_buffer_pool_manager();
            for (auto id : deleted_tb_id)
                EXPECT_EQ(nullptr, bpm->get_page((page_id_t)id)); // ensure the disk has deleted their pages
        }

        {
            // restart to ensure the delete operation has affected the disk
            DBManager db_mgr(meta);
            ASSERT_TRUE(db_mgr.get_status());

            Catalog *catalog = db_mgr.get_catalog();
            ASSERT_NE(nullptr, catalog);

            CatalogTable *catalog_table = catalog->get_catalog_table();
            ASSERT_NE(nullptr, catalog_table);

            // check
            EXPECT_EQ(tb_num - deleted_idx.size(), catalog_table->get_table_num());

            EXPECT_EQ(after_delete_free_space, catalog_table->get_free_space());

            for (auto i : deleted_tb_id)
                EXPECT_EQ(string_t(""), catalog_table->get_table_name(i));
            
            for (auto &name : deleted_tb_name)
                EXPECT_EQ(-1, catalog_table->get_table_id(name));

            BufferPoolManager *bpm = catalog->get_buffer_pool_manager();
            for (auto id : deleted_tb_id)
                EXPECT_EQ(nullptr, bpm->get_page((page_id_t)id)); // ensure the disk has deleted their pages
        }
    }

    for (int i = 0; i < tb_num; i++)
        delete table_schema[i];

    remove(meta_name);
    remove(db_name);
    remove(log_name);
}

} // namespace dawn
