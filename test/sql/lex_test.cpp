#include "gtest/gtest.h"
#include <vector>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <memory>

#include "util/util.h"
#include "sql/lex.h"
#include "sql/parse.h"
#include "test_util.h"

namespace dawn {

bool test0(const TestParam&);

const std::vector<bool(*)(const TestParam&)> tests{test0};

std::vector<int> parse_token(std::string file_path) {
    FILE* input_file = fopen(file_path.data(), "r");
    assert(input_file != nullptr);

    Lex lex(input_file);
    Token tk;

    std::vector<int> tokens;
    tokens.reserve(50);
    while (true) {
        if (!lex.next_token(&tk)) {
            break;
        }
        if (tk.type_ == TOKEN_ID || tk.type_ == TOKEN_STRING)
            delete[] tk.val_.varchar_;
        tokens.push_back(tk.type_);
    }

    return tokens;
}

bool test0(const TestParam& param) {
    const std::vector<int> target_tokens{
        TOKEN_CREATE, TOKEN_TABLE, TOKEN_ID, TOKEN_LEFT_PARENTHESES, TOKEN_ID,
        TOKEN_INT, TOKEN_COMMA, TOKEN_ID, TOKEN_CHAR, TOKEN_LEFT_PARENTHESES,
        TOKEN_INT_NUM, TOKEN_RIGHT_PARENTHESES, TOKEN_COMMA, TOKEN_ID,
        TOKEN_DECIMAL, TOKEN_COMMA, TOKEN_ID, TOKEN_BOOLEAN, TOKEN_COMMA,
        TOKEN_PRIMARY_KEY, TOKEN_LEFT_PARENTHESES, TOKEN_ID, TOKEN_RIGHT_PARENTHESES,
        TOKEN_RIGHT_PARENTHESES, TOKEN_SEMICOLON,

        TOKEN_DROP, TOKEN_TABLE, TOKEN_ID, TOKEN_SEMICOLON,

        TOKEN_INSERT, TOKEN_INTO, TOKEN_ID, TOKEN_VALUES, TOKEN_LEFT_PARENTHESES,
        TOKEN_INT_NUM, TOKEN_COMMA, TOKEN_STRING, TOKEN_COMMA, TOKEN_DECIMAL_NUM,
        TOKEN_COMMA, TOKEN_TRUE, TOKEN_RIGHT_PARENTHESES, TOKEN_SEMICOLON,

        TOKEN_DELETE, TOKEN_FROM, TOKEN_ID, TOKEN_WHERE, TOKEN_ID, TOKEN_LE_EQ,
        TOKEN_INT_NUM, TOKEN_SEMICOLON,

        TOKEN_SELECT, TOKEN_ID, TOKEN_FROM, TOKEN_ID, TOKEN_WHERE, TOKEN_ID,
        TOKEN_GREATER_SIGN, TOKEN_INT_NUM, TOKEN_SEMICOLON
    };
    
    const std::vector<int> parsed_tokens = parse_token(param.get_tcase());
    auto size = target_tokens.size();
    if (parsed_tokens.size() != size) {
        return false;
    }

    for (size_t i = 0; i < size; i++) {
        if (target_tokens[i] != parsed_tokens[i]) {
            return false;
        }
    }

    return true;
}

bool run_test(size_t test_num, const TestParam& test_param) {
    assert(test_num < tests.size());
    return tests[test_num](test_param);
}

TEST(ScannerTests, ScannerTest0) {
    TestParam tp("./test/lex_test0");
    bool success = run_test(0, tp);
    EXPECT_TRUE(success);
}
} // namespace dawn
