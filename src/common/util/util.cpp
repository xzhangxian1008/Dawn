#include <string>
#include <iostream>

namespace dawn {

/**
 * // FIXME can't be referenced by compiler
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

} // namespace dawn