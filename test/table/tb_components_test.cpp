#include "gtest/gtest.h"
#include "util/config.h"
#include "util/util.h"
#include "table/table.h"
#include "table/tuple.h"
#include "meta/catalog.h"
#include "meta/catalog_table.h"
#include "table/rid.h"
#include "table/table_schema.h"

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
string_t table("table4");
std::vector<TypeId> tb_col_types{TypeId::INTEGER, TypeId::CHAR, TypeId::BOOLEAN, TypeId::CHAR, TypeId::DECIMAL};
std::vector<string_t> tb_col_names{"tb4_col1", "tb4_col2", "tb4_col3", "tb4_col4", "tb4_col5"};
size_t_ tb_char0_sz = 5;
size_t_ tb_char1_sz = 10;
std::vector<size_t_> tb_char_size{tb_char0_sz, tb_char1_sz};
size_t_ tb_tuple_size = Type::get_integer_size() + tb_char0_sz + Type::get_bool_size() + tb_char1_sz + Type::get_decimal_size();

/**
 * Test List:
 *   1. test the basic functions of the Tuple object
 *   2. test the basic functions of ...
 *   3. test the basic functions of ...
 *   4. test the basic functions of ...
 */
TEST(TbComponentTest, BasicTest) {
    {
        // test 1

        // prepare for Value
        integer_t v0 = 2333;
        char v1[6] = "apple";
        boolean_t v2 = true;
        char v3[11] = "monkey_key";
        decimal_t v4 = 3.1415926;

        std::vector<Value> values;
        values.push_back(Value(v0));
        values.push_back(Value(v1, tb_char0_sz));
        values.push_back(Value(v2));
        values.push_back(Value(v3, tb_char1_sz));
        values.push_back(Value(v4));

        // prepare for RID
        page_id_t page_id = 123;
        offset_t slot_num = 111;
        RID rid(page_id, slot_num);

        // prepare for TableSchema
        TableSchema *table_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);

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
}

} // namespace dawn
