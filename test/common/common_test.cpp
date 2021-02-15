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
    std::set<Value> s;
    s.insert(Value(123));
    s.insert(Value(1234));
    s.insert(Value(1235));
    s.insert(Value(1236));
    s.insert(Value(1237));
    
    auto iter = s.find(Value(123));
    if (iter == s.end()) {
        LOG("here");
    }
    iter = s.find(Value(1234));
    if (iter == s.end()) {
        LOG("here");
    }
    iter = s.find(Value(1235));
    if (iter == s.end()) {
        LOG("here");
    }
    iter = s.find(Value(1236));
    if (iter == s.end()) {
        LOG("here");
    }
    iter = s.find(Value(1237));
    if (iter == s.end()) {
        LOG("here");
    }
}

} // namespace dawn