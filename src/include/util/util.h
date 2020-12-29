#include <iostream>

#define PRINT(...) print__(__VA_ARGS__)
// TODO logging

inline void print__() { std::cout << std::endl; };

template<typename T, typename... Types>
void print__(const T& firstArg, const Types&... args) {
    std::cout << firstArg << std::ends;
    print__(args...);
}
