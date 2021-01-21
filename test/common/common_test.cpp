#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "data/types.h"

using std::fstream;
using std::string;
using std::ios;
using std::set;
using std::cout;
using std::endl;
using std::ends;

namespace dawn {

union un {
    boolean_t boolean;
    integer_t integer;
    decimal_t decimal;
    char *char_;
};

TEST(CommonTest, CommonTEST) {
    un u;
    u.integer = 123;
    un u1;
    u1 = u;
}

} // namespace dawn