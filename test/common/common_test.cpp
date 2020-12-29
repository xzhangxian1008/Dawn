#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include "util/util.h"

namespace dawn {

TEST(DiskManagerTest, COMMON) {
    std::fstream f;

    f.open("ewqeqw", std::ios::in);
    if (f.is_open()) {
        PRINT("open suc");
        return;
    } else {
        PRINT("open fail");
    }

    f.clear();

    f.open("ewqeqw", std::ios::out);
    if (f.is_open()) {
        PRINT("create suc");
    } else {
        PRINT("create fail");
    }
}

} // namespace dawn