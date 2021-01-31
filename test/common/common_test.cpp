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

//全局
class GlobalTest:public testing::Environment {
public:
    virtual void SetUp(){cout<<"gtest introduction example. SetUp"<<endl;}
    virtual void TearDown(){cout<<"gtest introduction example. TearDown"<<endl;}
};

//在第一个test之前，最后一个test之后调用SetUpTestCase()和TearDownTestCase()
class CommonTest1:public testing::Test {
public:
    static void SetUpTestCase(){cout<<"Map. SetUpTestCase()"<<endl;}
    static void TearDownTestCase(){cout<<"Map. TearDownTestCase()"<<endl;}
    void SetUp() {cout<<"Map. SetUp()"<<endl;}
    void TearDown(){cout<<"Map. TearDown()"<<endl;}
};

class CommonTest2:public testing::Test {
public:
    static void SetUpTestCase(){cout<<"Fun. SetUpTestCase()"<<endl;}
    static void TearDownTestCase(){cout<<"Fun. TearDownTestCase()"<<endl;}
};

TEST_F(CommonTest1, CommonTEST11) {}
TEST_F(CommonTest1, CommonTEST12) {}
TEST_F(CommonTest2, CommonTEST21) {}
TEST_F(CommonTest2, CommonTEST22) {}

} // namespace dawn