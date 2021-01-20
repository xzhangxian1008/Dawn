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

string_t f() {
    un v;
    char str[10];
    str[0] = '1';
    str[1] = '2';
    str[2] = '3';
    str[3] = '\0';
    v.char_ = str;
    return v.char_;
}

TEST(CommonTest, CommonTEST) {
    string_t str = f();
    PRINT(str);
}

} // namespace dawn