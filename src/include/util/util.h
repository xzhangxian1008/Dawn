#pragma once

#include <iostream>
#include <string>
#include <fstream>

namespace dawn {
#define PRINT(...) print__(__VA_ARGS__)

// TODO let log receives random number of parameters
#define LOG(info) log__(__func__, __LINE__, info)

inline void print__() { std::cout << std::endl; };

template<typename T, typename... Types>
void print__(const T& firstArg, const Types&... args) {
    std::cout << firstArg << " ";
    print__(args...);
}

// TODO add switch、time and so no
void log__(std::string func_name, int line, std::string info) {
    std::string out = func_name + ", line " + std::to_string(line) + ": " + info;
    std::cout << out << std::endl;
}

char* string2char(const std::string &str);
bool check_inexistence(const std::string &file_name);
bool open_file(const std::string &file_name, std::fstream &io, std::ios_base::openmode om);
long get_file_sz(std::fstream &io);

} // namespace dawn
