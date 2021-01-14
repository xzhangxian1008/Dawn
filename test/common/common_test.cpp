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
    const char *meta_name = "file";
    fstream_t meta_io;
    if (!open_file(meta_name, meta_io, ios::in | ios::out | ios::trunc)) {
        LOG("can't create");
        return;
    }
    meta_io.seekg(0);
    const char *buf = "123456789";
    meta_io.write(buf, 10);
    meta_io.seekp(0);
    char rbuf[20];
    meta_io.read(rbuf, 5);
    if (meta_io.fail()) {
        LOG("fail");
    }
    meta_io.seekp(0);
    meta_io.read(rbuf, 20);
    if (meta_io.fail()) {
        LOG("fail");
    }
}

} // namespace dawn