#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include "util/util.h"
#include "util/config.h"

using std::fstream;
using std::string;
using std::ios;

namespace dawn {

TEST(CommonTest, CommonTEST) {
    fstream f;
    f.open("123", ios::in);
    f.close();
}

} // namespace dawn