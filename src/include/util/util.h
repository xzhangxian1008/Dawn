#pragma once

#include <iostream>
#include <string>
#include <fstream>

#include "murmur3/MurmurHash3.h"
#include "util/config.h"

namespace dawn {

#define DISALLOW_COPY(cname)                   \
    cname(const cname &) = delete;             \
    cname &operator=(const cname &) = delete;

#define DISALLOW_MOVE(cname)              \
    cname(cname &&) = delete;             \
    cname &operator=(cname &&) = delete; 

#define DISALLOW_COPY_AND_MOVE(cname)   \
    DISALLOW_COPY(cname);               \
    DISALLOW_MOVE(cname);

#define PRINT(...) print__(__VA_ARGS__)

// TODO let log receives random number of parameters
#define LOG(info) log__(__FILE__, __func__, __LINE__, info)

#define FATAL(info) fatal__(__FILE__, __func__, __LINE__, info)

inline void print__() { std::cout << std::endl; };

/**
 * TODO change the output to a thread safe method
 * do not use cout to output with this method in concurrency environment
 * This is thread unsafety
 */
template<typename T, typename... Types>
void print__(const T& firstArg, const Types&... args) {
    std::cout << firstArg << " ";
    print__(args...);
}

// TODO add switch、time and so no
inline void log__(std::string file_name, std::string func_name, int line, std::string info) {
    std::string out = file_name + " " + func_name + ", line " + std::to_string(line) + ": " + info;
    std::cout << out << std::endl;
}

inline void fatal__(std::string file_name, std::string func_name, int line, std::string info) {
    std::string out = file_name + " " + func_name + ", line " + std::to_string(line) + ": " + info;
    std::cout << out << std::endl;
    exit(-1);
}

inline hash_t do_hash(void *key, size_t_ key_size) {
    hash_t hash[2];
    murmur3::MurmurHash3_x64_128(reinterpret_cast<const void *>(key), key_size, 0,
                                reinterpret_cast<void *>(&hash));
    return hash[0];
}

char* string2char(const std::string &str);
bool check_inexistence(const std::string &file_name);
bool open_file(const std::string &file_name, std::fstream &io, std::ios_base::openmode om);
long get_file_sz(std::fstream &io);
void fill_char_array(const std::string &str, char* char_array);

} // namespace dawn
