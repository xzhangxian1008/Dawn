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

/**
 * ensure the file is inexistent
 * @return true: not exist  false: exist
 */
inline bool check_inexistence(const std::string &file_name) {
    std::fstream f;
    f.open(file_name, std::ios::in);
    if (f.is_open()) {
        f.close();
        return false;
    }
    return true;
}

/**
 * open file with given open mode
 * @return true: successful  false: unsuccessful
 */
inline bool open_file(const std::string &file_name, std::fstream &io, std::ios_base::openmode om) {
    io.open(file_name, om);
    if (!io.is_open()) {
        return false;
    }
    return true;
}

} // namespace dawn
