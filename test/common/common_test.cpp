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

class Base {
public:
    Base() = default;
    virtual ~Base() {};
    virtual void func() = 0;
};

class Derived : public Base {
public:
    ~Derived() override {}
    void func() override {}
    int x;
};

TEST(CommonTest, CommonTEST) {
    Base *b = new Derived;
    delete b;
}

} // namespace dawn