#include <string>
#include <iostream>
#include <fstream>

// #include "util/util.h"

namespace dawn {

/**
 * ATTENTION don't forget to delete the char* after use !!!
 */
char* string2char(const std::string &str) {
    int len = str.length();
    char *s = new char[len+1];
    for (int i = 0; i < len; i++)
        s[i] = str[i];
    s[len] = '\0';
    return s;
}

/**
 * ensure the file is inexistent
 * @return true: not exist  false: exist
 */
bool check_inexistence(const std::string &file_name) {
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
bool open_file(const std::string &file_name, std::fstream &io, std::ios_base::openmode om) {
    io.open(file_name, om);
    if (!io.is_open()) {
        return false;
    }
    return true;
}

long get_file_sz(std::fstream &io) {
    if (!io.is_open()) {
        return -1;
    }
    io.seekp(0, std::ios::beg);
    long beg = io.tellp();
    if (beg == -1) {
        return -1;
    }
    io.seekp(0, std::ios::end);
    return io.tellp() - beg;
}

} // namespace dawn