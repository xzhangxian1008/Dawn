#include "data/types.h"
#include "data/values.h"
#include "util/config.h"
#include "gtest/gtest.h"

namespace dawn {

TEST(ValueTest, Test1) {
    Value b_true(true);
    Value b_false(false);

    EXPECT_EQ(CmpResult::FALSE, CMP_EQ(BOOLEAN_T, b_true, b_false));
    b_true.swap(b_false);
    EXPECT_EQ(false, b_true.get_value<boolean_t>());
    EXPECT_EQ(true, b_false.get_value<boolean_t>());

    Value integer_1(111);
    Value integer_2(222);

    EXPECT_EQ(CmpResult::FALSE, CMP_EQ(INTEGER_T, integer_1, integer_2));
    EXPECT_EQ(CmpResult::TRUE, CMP_LESS(INTEGER_T, integer_1, integer_2));
    EXPECT_EQ(CmpResult::TRUE, CMP_LESS_EQ(INTEGER_T, integer_1, integer_2));
    EXPECT_EQ(CmpResult::FALSE, CMP_GREATER(INTEGER_T, integer_1, integer_2));
    EXPECT_EQ(CmpResult::FALSE, CMP_GREATER_EQ(INTEGER_T, integer_1, integer_2));

    
}

} // namespace dawn
