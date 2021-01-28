#include "gtest/gtest.h"
#include "util/config.h"
#include "util/util.h"
#include "table/table.h"
#include "table/tuple.h"
#include "meta/catalog.h"
#include "meta/catalog_table.h"
#include "table/rid.h"
#include "table/table_schema.h"
#include "manager/db_manager.h"

namespace dawn {
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
string_t table_name("table4");
std::vector<TypeId> tb_col_types{TypeId::INTEGER, TypeId::CHAR, TypeId::BOOLEAN, TypeId::CHAR, TypeId::DECIMAL};
std::vector<string_t> tb_col_names{"tb4_col1", "tb4_col2", "tb4_col3", "tb4_col4", "tb4_col5"};
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

class TbComponentTest : public testing::Test {
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
 *   1. test the basic functions of the Tuple object
 *   2. test the basic functions of the TablePage object
 *          ATTENTION tuples inserted in the TablePage should be fixed and same size in test 2
 *   3. test the basic functions of the Table object
 */

// test 1
TEST(TbComponentTest, TupleBasicTest) {
    // prepare for Value
    v0 = 2333;
    fill_char_array("apple", v1);
    v2 = true;
    fill_char_array("monkey_key", v3);
    v4 = 3.1415926;
    
    values.push_back(Value(v0));
    values.push_back(Value(v1, tb_char0_sz));
    values.push_back(Value(v2));
    values.push_back(Value(v3, tb_char1_sz));
    values.push_back(Value(v4));

    // prepare for RID
    page_id_t page_id = 123;
    offset_t slot_num = 111;
    RID rid(page_id, slot_num);

    TableSchema *table_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);

    // create Tuple
    Tuple tuple(&values, *table_schema, rid);

    ASSERT_TRUE(tuple.is_allocated());
    EXPECT_NE(nullptr, tuple.get_data());
    EXPECT_EQ(table_schema->get_tuple_size(), tuple.get_size());
    EXPECT_TRUE(rid == tuple.get_rid());

    // ensure the tuple to contain the correct values
    for (size_t i = 0; i < values.size(); i++)
        EXPECT_TRUE(values[i] == tuple.get_value(*table_schema, i));

    // change tuple's rid and ensure the change works
    page_id = 2333;
    slot_num = 777;
    RID another_rid(page_id, slot_num);
    tuple.set_rid(another_rid);
    EXPECT_TRUE(another_rid == tuple.get_rid());

    // change tuple's value and ensure the change works
    v0 = 66666;
    fill_char_array(string_t("Make"), v1);
    v2 = false;
    fill_char_array(string_t("for_win!"), v3);
    v4 = 2.718281;
    values.clear();
    values.push_back(Value(v0));
    values.push_back(Value(v1, tb_char0_sz));
    values.push_back(Value(v2));
    values.push_back(Value(v3, tb_char1_sz));
    values.push_back(Value(v4));

    for (size_t i = 0; i < values.size(); i++)
        tuple.set_value(*table_schema, &(values[i]), i);

    // check the change works
    for (size_t i = 0; i < values.size(); i++) {
        if (values[i] == tuple.get_value(*table_schema, i))
            continue;
        EXPECT_TRUE(values[i] == tuple.get_value(*table_schema, i));
    }
        
    delete table_schema;
}

// test 2
TEST(TbComponentTest, TablePageBasicTest) {
    TableSchema *table_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);

    DBManager *db_manager = new DBManager(meta, true);
    ASSERT_TRUE(db_manager->get_status());
    
    BufferPoolManager *bpm = db_manager->get_buffer_pool_manager();

    // get TablePage
    Page *page = bpm->new_page();
    TablePage *table_page = reinterpret_cast<TablePage*>(page);
    page_id_t page_id = table_page->get_page_id();
    table_page->init(INVALID_PAGE_ID, INVALID_PAGE_ID);

    //==------------------------------------------------------------------------==//
    /** check the initial values, set and recheck */
    EXPECT_EQ(INVALID_PAGE_ID, table_page->get_next_page_id());
    EXPECT_EQ(INVALID_PAGE_ID, table_page->get_prev_page_id());

    const page_id_t next_page_id = 99;
    const page_id_t prev_page_id = 98;
    table_page->set_next_page_id(next_page_id);
    table_page->set_prev_page_id(prev_page_id);
    EXPECT_EQ(next_page_id, table_page->get_next_page_id());
    EXPECT_EQ(prev_page_id, table_page->get_prev_page_id());

    //==------------------------------------------------------------------------==//
    /** check single tuple's operation */
    v0 = 2333;
    fill_char_array("apple", v1);
    v2 = true;
    fill_char_array("monkey_key", v3);
    v4 = 3.1415926;

    RID inserted_pos;

    {
        /** insert and check */

        values.push_back(Value(v0));
        values.push_back(Value(v1, tb_char0_sz));
        values.push_back(Value(v2));
        values.push_back(Value(v3, tb_char1_sz));
        values.push_back(Value(v4));

        Tuple tuple(&values, *table_schema);
        ASSERT_TRUE(tuple.is_allocated());
        EXPECT_TRUE(table_page->insert_tuple(tuple, &inserted_pos));
        tuple.set_rid(inserted_pos);

        Tuple tuple_container;
        EXPECT_TRUE(table_page->get_tuple(&tuple_container, inserted_pos));
        EXPECT_TRUE(tuple == tuple_container);

        bool ok = true;
        for (size_t_ i = 0; i < table_schema->get_column_num(); i++) {
            Value v1 = tuple.get_value(*table_schema, i);
            Value v2 = tuple_container.get_value(*table_schema, i);
            if (v1 == v2)
                continue;
            ok = false;
            break;
        }
        EXPECT_TRUE(ok);
    }

    {
        /** update tuple and check */

        v0 = 2333;
        fill_char_array(string_t("Make"), v1);
        v2 = true;
        fill_char_array(string_t("for_win!"), v3);
        v4 = 3.1415926;

        values.clear();
        values.push_back(Value(v0));
        values.push_back(Value(v1, tb_char0_sz));
        values.push_back(Value(v2));
        values.push_back(Value(v3, tb_char1_sz));
        values.push_back(Value(v4));

        Tuple new_tuple(&values, *table_schema, inserted_pos);
        Tuple tuple_container;
        EXPECT_TRUE(table_page->update_tuple(new_tuple, inserted_pos));
        EXPECT_TRUE(table_page->get_tuple(&tuple_container, inserted_pos));
        EXPECT_TRUE(new_tuple == tuple_container);

        bool ok = true;
        for (size_t_ i = 0; i < table_schema->get_column_num(); i++) {
            Value v1 = new_tuple.get_value(*table_schema, i);
            Value v2 = tuple_container.get_value(*table_schema, i);
            if (v1 == v2)
                continue;
            ok = false;
            break;
        }
        EXPECT_TRUE(ok);
    }

    {
        /** invalid update and check */

        RID rid(100, 100); // all invalid
        Tuple tuple;
        EXPECT_FALSE(table_page->update_tuple(tuple, rid));

        rid.set(page_id, 100); // invalid slot_num
        EXPECT_FALSE(table_page->update_tuple(tuple, rid));

        rid.set(100, inserted_pos.get_slot_num()); // invalid page_id
        EXPECT_FALSE(table_page->update_tuple(tuple, rid));
    }

    {
        /** invalid deletion and check */

        RID rid(100, 100); // all invalid
        Tuple tuple;
        EXPECT_FALSE(table_page->mark_delete(rid));

        rid.set(page_id, 100); // invalid slot_num
        EXPECT_FALSE(table_page->mark_delete(rid));

        rid.set(100, inserted_pos.get_slot_num()); // invalid page_id
        EXPECT_FALSE(table_page->mark_delete(rid));
    }
    
    {
        /** delete tuple and check */

        EXPECT_TRUE(table_page->mark_delete(inserted_pos));
        table_page->apply_delete(inserted_pos);

        Tuple tuple_container;
        EXPECT_FALSE(table_page->get_tuple(&tuple_container, inserted_pos));
    }

    //==------------------------------------------------------------------------==//
    /** check many many tuples' operation */

    const size_t_ max_inserted_num = 
        TablePage::get_tp_load_data_space() / (table_schema->get_tuple_size() + table_page->get_tuple_record_sz());

    {
        /** insert max_inserted_num and check */

        values.clear();
        v0 = 0;
        fill_char_array("apple", v1);
        v2 = true;
        fill_char_array("monkey_key", v3);
        v4 = 3.1415926;

        values.push_back(Value(v0));
        values.push_back(Value(v1, tb_char0_sz));
        values.push_back(Value(v2));
        values.push_back(Value(v3, tb_char1_sz));
        values.push_back(Value(v4));

        bool ok = true;
        for (size_t_ i = 0; i < max_inserted_num; i++) {
            values[0] = Value(i);
            Tuple tuple(&values, *table_schema);

            if (!table_page->insert_tuple(tuple, &inserted_pos)) {
                ok = false;
                break;
            }
        }
        EXPECT_TRUE(ok);

        {
            // insert should fail
            values[0] = Value(max_inserted_num);
            Tuple tuple(&values, *table_schema);
            EXPECT_FALSE(table_page->insert_tuple(tuple, &inserted_pos));
        }

        // check tuples have been inserted successfully
        ok = true;
        for (size_t_ i = 0; i < max_inserted_num; i++) {
            values[0] = Value(i);
            Tuple tuple(&values, *table_schema);
            Tuple tuple_container;
            inserted_pos.set(page_id, i);
            tuple.set_rid(inserted_pos);
            if (!table_page->get_tuple(&tuple_container, inserted_pos)) {
                ok = false;
                break;
            }

            if (!(tuple == tuple_container)) {
                ok = false;
                break;
            }

            for (size_t_ i = 0; i < table_schema->get_column_num(); i++) {
                Value v1 = tuple.get_value(*table_schema, i);
                Value v2 = tuple_container.get_value(*table_schema, i);
                if (v1 == v2)
                    continue;
                ok = false;
                break;
            }
            if (!ok)
                break;
        }
        EXPECT_TRUE(ok);
    }

    int deleted_num = 0;
    {
        /** delete many tuples and check */
        values.clear();
        v0 = 0;
        fill_char_array("apple", v1);
        v2 = true;
        fill_char_array("monkey_key", v3);
        v4 = 3.1415926;

        values.push_back(Value(v0));
        values.push_back(Value(v1, tb_char0_sz));
        values.push_back(Value(v2));
        values.push_back(Value(v3, tb_char1_sz));
        values.push_back(Value(v4));

        bool ok = true;

        // delete
        for (size_t_ i = 0; i < max_inserted_num; i++) {
            if (i % 2 == 0)
                continue;
            RID rid(page_id, i);
            if (!table_page->mark_delete(rid)) {
                ok = false;
                break;
            }
            table_page->apply_delete(rid);
            deleted_num++;
        }
        EXPECT_TRUE(ok);

        // check
        ok = true;
        for (size_t_ i = 0; i <max_inserted_num; i++) {
            RID rid(page_id, i);
            Tuple tuple;
            bool result = table_page->get_tuple(&tuple, rid);
            if (i % 2 != 0 && result) {
                ok = false;
                break;
            } else if (i % 2 == 0) {
                if (!result) {
                    ok = false;
                    break;
                }

                // check the correctness of the existing tuples
                values[0] = Value(i);
                Tuple tuple_valid(&values, *table_schema, rid);
                if (!(tuple == tuple_valid)) {
                    ok = false;
                    break;
                }
            }
        }
        EXPECT_TRUE(ok);
    }

    {
        /** reinsert tuples and check */
        values.clear();
        v0 = 0;
        fill_char_array("apple", v1);
        v2 = true;
        fill_char_array("monkey_key", v3);
        v4 = 3.1415926;

        values.push_back(Value(v0));
        values.push_back(Value(v1, tb_char0_sz));
        values.push_back(Value(v2));
        values.push_back(Value(v3, tb_char1_sz));
        values.push_back(Value(v4));

        bool ok = true;
        for (size_t_ i = 0; i < deleted_num; i++) {
            values[0] = Value(i);
            Tuple tuple(&values, *table_schema);

            if (!table_page->insert_tuple(tuple, &inserted_pos)) {
                ok = false;
                break;
            }
        }
        EXPECT_TRUE(ok);

        {
            // insert should fail
            values[0] = Value(deleted_num);
            Tuple tuple(&values, *table_schema);
            EXPECT_FALSE(table_page->insert_tuple(tuple, &inserted_pos));
        }
    }

    delete table_schema;
    delete db_manager;
}

// test 3
TEST(TbComponentTest, TableBasicTest) {
    TableSchema *table_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);

    size_t_ insert_num = 56789;
    std::vector<Tuple> insert_tuples;

    {
        /** insert a lot of tuples and check */
        DBManager *db_manager = new DBManager(meta, true);
        ASSERT_TRUE(db_manager->get_status());

        Catalog *catalog = db_manager->get_catalog();
        CatalogTable *catalog_table = catalog->get_catalog_table();
        ASSERT_TRUE(catalog_table->create_table(table_name, *table_schema));

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
        values.push_back(v0);
        values.push_back(v1);
        values.push_back(v2);
        values.push_back(v3);
        values.push_back(v4);

        // insert a lot of tuples
        bool ok = true;
        RID rid;
        for (size_t_ i = 0; i < insert_num; i++) {
            values[0] = Value(static_cast<integer_t>(i));
            Tuple tuple(&values, *table_schema);
            PRINT("start insert", i);
            if (!table->insert_tuple(tuple, &rid)) {
                ok = false;
            }
            PRINT("insert", i, "successfully");
            tuple.set_rid(rid);
            insert_tuples.push_back(tuple);
        }
        EXPECT_TRUE(ok);

        // check
        ok = true;
        Tuple tuple_container;
        for (auto tuple : insert_tuples) {
            if (!table->get_tuple(&tuple_container, tuple.get_rid())) {
                ok = false;
                break;
            }

            if (!(tuple_container == tuple)) {
                ok = false;
                break;
            }
        }
        EXPECT_TRUE(ok);

        delete db_manager;
    }

    delete table_schema;
}

} // namespace dawn
