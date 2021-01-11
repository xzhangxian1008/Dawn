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

class C{
public:
    C(int x) : x_(x) {
        PRINT("constructor");
    }
    C(const C &c) {
        this->x_ = c.x_;
        PRINT("copy");
    }
    const C& operator=(const C &c) {
        PRINT("operator=");
        return *this;
    }
    int x_;
};

void foo(C c) {}

C func(C c) {
    return c;
}

TEST(CommonTest, CommonTEST) {
    C c = 10;
    C cc(c);
}

} // namespace dawn