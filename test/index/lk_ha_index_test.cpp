#include "gtest/gtest.h"
#include "table/table_schema.h"
#include "manager/db_manager.h"
#include "table/table.h"

namespace dawn {

extern DBManager *db_manager;

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
string_t table_name("table");
std::vector<TypeId> tb_col_types{TypeId::INTEGER, TypeId::CHAR, TypeId::BOOLEAN, TypeId::CHAR, TypeId::DECIMAL};
std::vector<string_t> tb_col_names{"tb_col1", "tb_col2", "tb_col3", "tb_col4", "tb_col5"};
size_t_ tb_char0_sz = 5;
size_t_ tb_char1_sz = 10;
std::vector<size_t_> tb_char_size{tb_char0_sz, tb_char1_sz};
size_t_ tb_tuple_size = Type::get_integer_size() + tb_char0_sz + Type::get_bool_size() + tb_char1_sz + Type::get_decimal_size();

const char *meta = "test";
const char *mtdf = "test.mtd";
const char *dbf = "test.db";
const char *logf = "test.log";

integer_t v0;
char v1[6];
boolean_t v2;
char v3[11];
decimal_t v4;
std::vector<Value> values;

class LinkHashBasicTest : public testing::Test {
public:
    void SetUp() {
        values.clear();
        remove(mtdf);
        remove(dbf);
        remove(logf);
    }

    void TearDown() {
        remove(mtdf);
        remove(dbf);
        remove(logf);
    }
};

/**
 * Test List:
 *   1. insert a lot of tuples and check with index's search function
 *      restart the db to ensure the operation have been persisted on the disk
 *   2. update some tuples and check
 *      restart the db to ensure the operation have been persisted on the disk
 *   3. delete some tuples and check
 *      restart the db to ensure the operation have been persisted on the disk
 *   4. reinsert some tuples and check
 *      restart the db to ensure the operation have been persisted on the disk
 */
TEST_F(LinkHashBasicTest, BasicTest) {
    PRINT("start the hash link index tests...");
    TableSchema *tb_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);
    size_t_ insert_num = 12345;
    std::vector<Tuple> insert_tuples;
    
    {
        // test 1
        {
            db_manager = new DBManager(meta, true);
            ASSERT_TRUE(db_manager->get_status());
            
            Catalog *catalog = db_manager->get_catalog();
            CatalogTable *catalog_table = catalog->get_catalog_table();
            ASSERT_TRUE(catalog_table->create_table(table_name, *tb_schema));

            TableMetaData *table_md = catalog_table->get_table_meta_data(table_name);
            ASSERT_NE(nullptr, table_md);

            Table *table = table_md->get_table();
            ASSERT_NE(nullptr, table);

            v0 = 2333;
            fill_char_array("apple", v1);
            v2 = true;
            fill_char_array("monkey_key", v3);
            v4 = 3.1415926;

            values.clear();
            values.push_back(Value(v0));
            values.push_back(Value(v1, tb_char0_sz));
            values.push_back(Value(v2));
            values.push_back(Value(v3, tb_char1_sz));
            values.push_back(Value(v4));

            // insert a lot of tuples
            PRINT("insert a lot of tuples...");
            bool ok = true;
            for (size_t_ i = 0; i < insert_num; i++) {
                values[0] = Value(static_cast<integer_t>(i));
                Tuple tuple(&values, *tb_schema);
                if (!table->insert_tuple(&tuple, *tb_schema)) {
                    ok = false;
                    break;
                }
                insert_tuples.push_back(tuple);
            }
            EXPECT_TRUE(ok);

            // check the tuples
            ok = true;
            size_t_ size = insert_tuples.size();
            Tuple cmp_tuple;
            for (size_t_ i = 0; i < size; i++) {
                Value val = insert_tuples[i].get_value(*tb_schema, tb_schema->get_key_idx());
                if (!table->get_tuple(val, &cmp_tuple, *tb_schema)) {
                    ok = false;
                    break;
                }

                if (!(insert_tuples[i] == cmp_tuple)) {
                    ok = false;
                    break;
                }
            }
            ASSERT_TRUE(ok);
            PRINT("insert a lot of tuples successfully!");
            delete db_manager;
        }

        {
            PRINT("restart the db...");
            db_manager = new DBManager(meta);
            ASSERT_TRUE(db_manager->get_status());
            
            Catalog *catalog = db_manager->get_catalog();
            CatalogTable *catalog_table = catalog->get_catalog_table();

            table_id_t table_id = catalog_table->get_table_id(table_name);
            TableMetaData *table_md = catalog_table->get_table_meta_data(table_id);
            ASSERT_NE(table_md, nullptr);

            Table *table = table_md->get_table();
            ASSERT_NE(nullptr, table);

            // check the tuples
            bool ok = true;
            size_t_ size = insert_tuples.size();
            Tuple cmp_tuple;
            for (size_t_ i = 0; i < size; i++) {
                Value val = insert_tuples[i].get_value(*tb_schema, tb_schema->get_key_idx());
                if (!table->get_tuple(val, &cmp_tuple, *tb_schema)) {
                    ok = false;
                    break;
                }

                if (!(insert_tuples[i] == cmp_tuple)) {
                    ok = false;
                    break;
                }
            }
            ASSERT_TRUE(ok);
            PRINT("operation have been persisted on the disk successfully!");
            delete db_manager;
        }
    }


    {
        // test 2
        {
            db_manager = new DBManager(meta);
            ASSERT_TRUE(db_manager->get_status());
            
            Catalog *catalog = db_manager->get_catalog();
            CatalogTable *catalog_table = catalog->get_catalog_table();

            table_id_t table_id = catalog_table->get_table_id(table_name);
            TableMetaData *table_md = catalog_table->get_table_meta_data(table_id);
            ASSERT_NE(table_md, nullptr);

            Table *table = table_md->get_table();
            ASSERT_NE(nullptr, table);

            v0 = 2333;
            fill_char_array("apple", v1);
            v2 = true;
            fill_char_array("monkey_key", v3);
            v4 = 3.1415926;

            values.clear();
            values.push_back(Value(v0));
            values.push_back(Value(v1, tb_char0_sz));
            values.push_back(Value(v2));
            values.push_back(Value(v3, tb_char1_sz));
            values.push_back(Value(v4));

            PRINT("start to update some tuples...");
            bool ok = true;
            for (size_t_ i = 0; i < insert_num; i++) {
                if (i % 2 == 0)
                    continue;
                values[0] = Value(static_cast<integer_t>(i + insert_num)); // update the key
                Tuple tuple(&values, *tb_schema);
                if (!table->update_tuple(&tuple, insert_tuples[i].get_rid(), *tb_schema)) {
                    ok = false;
                    break;
                }
                insert_tuples[i] = tuple;
            }
            EXPECT_TRUE(ok);

            // check the tuples
            ok = true;
            size_t_ size = insert_tuples.size();
            Tuple cmp_tuple;
            for (size_t_ i = 0; i < size; i++) {
                Value val = insert_tuples[i].get_value(*tb_schema, tb_schema->get_key_idx());
                if (!table->get_tuple(val, &cmp_tuple, *tb_schema)) {
                    ok = false;
                    break;
                }

                if (!(insert_tuples[i] == cmp_tuple)) {
                    ok = false;
                    break;
                }
            }
            ASSERT_TRUE(ok);
            PRINT("update some tuples successfully!");
            delete db_manager;
        }

        {
            PRINT("restart the db...");
            db_manager = new DBManager(meta);
            ASSERT_TRUE(db_manager->get_status());
            
            Catalog *catalog = db_manager->get_catalog();
            CatalogTable *catalog_table = catalog->get_catalog_table();

            table_id_t table_id = catalog_table->get_table_id(table_name);
            TableMetaData *table_md = catalog_table->get_table_meta_data(table_id);
            ASSERT_NE(table_md, nullptr);

            Table *table = table_md->get_table();
            ASSERT_NE(nullptr, table);

            // check the 
            PRINT("check");
        }
    }

    delete tb_schema;
}

} // namespace dawn
