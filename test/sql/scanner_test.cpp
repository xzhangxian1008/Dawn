#include "gtest/gtest.h"
#include <vector>
#include <string>
#include <assert.h>
#include <unistd.h>

#include "util/util.h"
#include "sql/y.tab.h"
#include "test_util.h"

extern FILE* yyin;
extern char* lex_str;
int yylex();

namespace dawn {

bool test0(const TestParam&);

const std::vector<bool(*)(const TestParam&)> tests{test0};

std::vector<int> parse_token(std::string file_path) {
    yyin = fopen(file_path.data(), "r");
    assert(yyin != nullptr);

    std::vector<int> tokens;
    tokens.reserve(50);
    while (true) {
        int ret = yylex();
        if (ret == ID || ret == STRING)
            delete[] lex_str;
        if (ret == 0)
            break;
        tokens.push_back(ret);
    }

    return tokens;
}

bool test0(const TestParam& param) {
    const std::vector<int> target_tokens{
        CREATE, TABLE, ID, '(', ID, INT, ',', ID, CHAR, '(', INT_NUM, ')',
        ',', ID, DECIMAL, ',', ID, BOOLEAN, ',', PRIMARY_KEY, '(', ID, ')',
        ')', ';',
        DROP, TABLE, ID, ';',
        INSERT, INTO, ID, VALUES, '(', INT_NUM, ',', STRING, ',',
        FLOAT_NUM, ',', TRUE, ')', ';',
        DELETE, FROM, ID, WHERE, ID, '<', INT_NUM, ';',
        SELECT, ID, FROM, ID, WHERE, ID, '>', INT_NUM, ';'
    };
    
    const std::vector<int> parsed_tokens = parse_token(param.get_tcase());
    bool success = true;
    auto size = target_tokens.size();
    if (parsed_tokens.size() != size) {
        return false;
    }

    auto parsed_iter = parsed_tokens.begin();
    for (auto target : parsed_tokens) {
        if (*parsed_iter != target) {
            success = false;
            break;
        }
        parsed_iter++;
    }

    return success;
}

bool run_test(size_t test_num, const TestParam& test_param) {
    assert(test_num < tests.size());
    return tests[test_num](test_param);
}

TEST(ScannerTests, ScannerTest0) {
    TestParam tp("./test/scanner_test0");
    bool success = run_test(0, tp);
    EXPECT_TRUE(success);
}
} // namespace dawn
