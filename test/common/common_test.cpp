#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "data/types.h"
#include "data/values.h"


using std::fstream;
using std::string;
using std::ios;
using std::set;
using std::cout;
using std::endl;
using std::ends;

namespace dawn {

TEST(CommonTest1, CommonTEST11) {
    Value v(static_cast<integer_t>(1));
    Value v1(static_cast<integer_t>(1));
    Value v2(static_cast<integer_t>(1));
    Value v3(static_cast<integer_t>(1));

    PRINT(v.get_hash_value());
    PRINT(v.get_hash_value());
    PRINT(v1.get_hash_value());
    PRINT(v2.get_hash_value());
    PRINT(v3.get_hash_value());
}

} // namespace dawn