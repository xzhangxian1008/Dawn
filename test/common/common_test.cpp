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

class Val {
public:
    int x;
    Val(int x_) : x(x_) {}
    Val(Val &val) {
        PRINT("cp cons");
        x = val.x;
    }
    Val& operator=(const Val &val) {
        PRINT("op=");
        x = val.x;
        return *this;
    }
    Val& operator+(const Val &val) {
        this->x += val.x;
        return *this;
    }
};

TEST(CommonTest1, CommonTEST11) {
    Val v1(1);
    Val v2(2);
    Val v3(0);
    v3 = v1+v2;
    v3 + v2;
}

} // namespace dawn