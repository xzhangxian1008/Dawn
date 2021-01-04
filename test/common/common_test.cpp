#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include <set>

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"

using std::fstream;
using std::string;
using std::ios;
using std::set;
using std::cout;
using std::endl;
using std::ends;

namespace dawn {


TEST(CommonTest, CommonTEST) {
    Page *p;
    p = new Page[10];
    delete p;
}

} // namespace dawn