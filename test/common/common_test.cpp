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


TEST(CommonTest, CommonTEST) {
    char c[30] = "1234567890qwertyuiopasdfghjkl";
    memcpy(c + 5, c, 10);
    cout << c << endl;
}

} // namespace dawn