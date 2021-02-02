#include "data/types.h"
#include "data/values.h"
#include "util/config.h"
#include "gtest/gtest.h"
#include <iostream>
namespace dawn {

#define GETVALUE_D get_value<decimal_t>
#define MINUS_D(decimal_1, decimal_2) MINUS(DECIMAL_T, decimal_1, decimal_2)
#define MULTIPLY_D(decimal_1, decimal_2) MULTIPLY(DECIMAL_T, decimal_1, decimal_2)
#define DIVIDE_D(decimal_1, decimal_2) DIVIDE(DECIMAL_T, decimal_1, decimal_2)
#define MIN_D(decimal_1, decimal_2) MIN(DECIMAL_T, decimal_1, decimal_2)
#define MAX_D(decimal_1, decimal_2) MAX(DECIMAL_T, decimal_1, decimal_2)

// TODO test serialize and deserialize function
TEST(ValueTest, BasicTest) {
    {
        // test BOOLEAN
        Value b_true(true);
        Value b_false(false);

        EXPECT_EQ(CmpResult::FALSE, CMP_EQ(BOOLEAN_T, b_true, b_false));
        b_true.swap(b_false);
        EXPECT_EQ(false, b_true.get_value<boolean_t>());
        EXPECT_EQ(true, b_false.get_value<boolean_t>());
    }

    {
        // test INTEGER
        int v1 = 111;
        int v2 = 222;
        Value integer_1(v1);
        Value integer_2(v2);

        EXPECT_EQ(CmpResult::FALSE, CMP_EQ(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(CmpResult::TRUE, CMP_LESS(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(CmpResult::TRUE, CMP_LESS_EQ(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(CmpResult::FALSE, CMP_GREATER(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(CmpResult::FALSE, CMP_GREATER_EQ(INTEGER_T, integer_1, integer_2));

        Value val;

        val.load(MINUS(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1-v2).get_value<integer_t>(), val.get_value<integer_t>());

        val.load(ADD(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1+v2).get_value<integer_t>(), val.get_value<integer_t>());

        val.load(MULTIPLY(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1*v2).get_value<integer_t>(), val.get_value<integer_t>());

        val.load(DIVIDE(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1/v2).get_value<integer_t>(), val.get_value<integer_t>());

        val.load(MIN(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1).get_value<integer_t>(), val.get_value<integer_t>());

        val.load(MAX(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v2).get_value<integer_t>(), val.get_value<integer_t>());
        //test for serialization and deserialization
        char storage[DECIMAL_T_SIZE];
        integer_1.serialize_to(storage);
        integer_1.deserialize_from(storage);
    }

    {
        // test DECIMAL
        decimal_t v1 = 1.5;
        decimal_t v2 = 3.5;
        Value decimal_1(v1);
        Value decimal_2(v2);
        Value val;
        // minus
        val.load(MINUS_D(decimal_1, decimal_2));
        EXPECT_EQ(Value(v1 - v2).GETVALUE_D(), val.GETVALUE_D());
        //multply
        val.load(MULTIPLY_D(decimal_1, decimal_2));
        EXPECT_EQ(Value(v1 * v2).GETVALUE_D(), val.GETVALUE_D());

        //division
        val.load(DIVIDE_D(decimal_1, decimal_2));
        EXPECT_EQ(Value(v1 / v2).GETVALUE_D(), val.GETVALUE_D());

        //min
        val.load(MIN_D(decimal_1, decimal_2));
        EXPECT_EQ(Value(v1).GETVALUE_D(), val.GETVALUE_D());
        //max
        val.load(MAX_D(decimal_1, decimal_2));
        EXPECT_EQ(Value(v2).GETVALUE_D(), val.GETVALUE_D());
        //test serialize and deserialize method
        char storage[DECIMAL_T_SIZE];
        decimal_1.serialize_to(storage);
        decimal_1.deserialize_from(storage);
    }

    {
        //test CHAR
        char char_test_1[7] = "Kabuto";
        Value value_char_1(char_test_1, 6);
        Value value_char_2;
        // swap
        value_char_2.load(value_char_1);
        // ASSERT_TRUE(value_char_1 == value_char_2);
        //str_size
        int i = value_char_1.get_char_size();
        PRINT("char_size -> ", i);
        //get string
        string_t value_str = value_char_1.get_value_string();
        PRINT("string -> ", value_str);
    }
}

} // namespace dawn
