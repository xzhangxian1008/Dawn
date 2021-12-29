#pragma once

#include <string>

class TestParam {
public:
    TestParam(std::string tcase) : tcase_(tcase) {}
    std::string get_tcase() const { return tcase_; }
private:
    std::string tcase_; // test case file
};

