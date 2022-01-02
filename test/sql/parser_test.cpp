#include "gtest/gtest.h"
#include <assert.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>

extern FILE* yyin;
int yyparse();

namespace dawn {

TEST(ParserTests, ParserTest0) {
    std::string file_path("./test/parser_test0");
    yyin = fopen(file_path.data(),"r");
    assert(yyin != nullptr);

    EXPECT_EQ(yyparse(), 0);
}

TEST(ParserTests, ParserTest1) {
    std::string file_path("./test/parser_test1");
    yyin = fopen(file_path.data(),"r");
    assert(yyin != nullptr);

    EXPECT_EQ(yyparse(), 0);

    
}

} // namespace dawn
