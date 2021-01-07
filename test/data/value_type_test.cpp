#include "data/types.h"
#include "data/values.h"
#include "util/config.h"
#include "gtest/gtest.h"

namespace dawn {

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
        EXPECT_EQ(Value(v1-v2).get_value<integet_t>(), val.get_value<integet_t>());

        val.load(ADD(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1+v2).get_value<integet_t>(), val.get_value<integet_t>());

        val.load(MULTIPLY(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1*v2).get_value<integet_t>(), val.get_value<integet_t>());

        val.load(DIVIDE(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1/v2).get_value<integet_t>(), val.get_value<integet_t>());

        val.load(MIN(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v1).get_value<integet_t>(), val.get_value<integet_t>());

        val.load(MAX(INTEGER_T, integer_1, integer_2));
        EXPECT_EQ(Value(v2).get_value<integet_t>(), val.get_value<integet_t>());
    }
}

} // namespace dawn
