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

const char *s = "asdfe";

namespace dawn {

string_t func() {
    return s;
}

TEST(CommonTest1, CommonTEST11) {
    string_t str = func();
    PRINT(str);
}

} // namespace dawn