#include "sql/lex.h"

namespace dawn {

bool Lex::next_token(Token* tk) {
    char c = next_char();
    while (c == ' ' || c== '\n') {
        c = next_char();
    }

    if (c == 0) return false;

    switch (c) {
    case '(': return LEFT_PARENTHESES;
    case ')': return RIGHT_PARENTHESES;
    case ';': return SEMICOLON;
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '_':
    
    
    default:
        break;
    }
}
    
} // namespace dawn
