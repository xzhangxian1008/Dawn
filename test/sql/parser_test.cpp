#include "gtest/gtest.h"
#include <assert.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>

#include "ast/ddl.h"

extern FILE* yyin;
int yyparse();
extern dawn::StmtListNode* ast_root;

namespace dawn {

TEST(ParserTests, ParserTest0) {
    std::string file_path("./test/parser_test0");
    yyin = fopen(file_path.data(),"r");
    assert(yyin != nullptr);

    EXPECT_EQ(yyparse(), 0);
    delete ast_root;
}

TEST(ParserTests, ParserTest1) {
    std::string file_path("./test/parser_test1");
    yyin = fopen(file_path.data(),"r");
    assert(yyin != nullptr);

    EXPECT_EQ(yyparse(), 0);

    // TODO test if we create the table successfully
    delete ast_root;
}

} // namespace dawn
