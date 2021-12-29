#include "gtest/gtest.h"
#include <assert.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>

#include "test_util.h"

extern FILE* yyin;
int yyparse();

namespace dawn {
bool test0(const TestParam&);
const std::vector<bool(*)(const TestParam&)> tests{test0};

bool test0(const TestParam& param) {
    std::string file_path(param.get_tcase());
    yyin = fopen(file_path.data(),"r");
    assert(yyin != nullptr);

    if (yyparse() == 0) {
        return true;
    }
    return false;
}

bool run_test(size_t test_num, const TestParam& test_param) {
    assert(test_num < tests.size());
    return tests[test_num](test_param);
}

TEST(ParserTests, ParserTest0) {
    TestParam tp("./test/scanner_test0");
    bool success = run_test(0, tp);
    EXPECT_TRUE(success);
}

} // namespace dawn
