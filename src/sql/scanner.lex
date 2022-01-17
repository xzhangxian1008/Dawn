%{
#include <string>
#include <cstring>
#include "y.tab.h"
#include "util/config.h"

dawn::varchar_t lex_str;
dawn::integer_t int_num;
dawn::decimal_t float_num;
%}

digit   [0-9]
letter  [A-Za-z]
id      ({letter}|_)({letter}|{digit}|_)*
integer [1-9]{digit}*|0
float   {integer}\.{digit}*
string  '.*'
%%

"natural" {return NATURAL;}
"join" {return JOIN;}
"create" {return CREATE;}
"drop" {return DROP;}
"insert" {return INSERT;}
"delete" {return DELETE;}
"select" {return SELECT;}
"table" {return TABLE;}
"into" {return INTO;}
"values" {return VALUES;}
"from" {return FROM;}
"primary key" {return PRIMARY_KEY;}
"where" {return WHERE;}
"char" {return CHAR;}
"int" {return INT;}
"decimal" {return DECIMAL;}
"boolean" {return BOOLEAN;}
"true" {return TRUE;}
"false" {return FALSE;}
"not" {return NOT;}
">" {return '>';}
"<" {return '<';}
">=" {return GT_EQ;}
"<=" {return LE_EQ;}
"!=" {return NOT_EQ;}
"=" {return '=';}
"(" {return '(';}
")" {return ')';}
"{" {return '{';}
"}" {return '}';}
";" {return ';';}
"," {return ',';}
"||" {return OR;}
"&&" {return AND;}
{id}    {
            lex_str = new char[yyleng+1]; // ATTENTION! BE CAREFUL OF  THEMEMORY LEAK!
            std::memcpy(lex_str, yytext, yyleng);
            lex_str[yyleng] = 0;
            return ID;
        }

{integer} {
            int_num = atoi(yytext);
            return INT_NUM;
        }

{float} {
            float_num = atof(yytext);
            return FLOAT_NUM;
        }

{string} {
            lex_str = new char[yyleng-1]; // ATTENTION! BE CAREFUL OF  THEMEMORY LEAK!
            std::memcpy(lex_str, yytext+1, yyleng-2);
            lex_str[yyleng-2] = 0;
            return STRING;
        }
%%

int yywrap(void) {
    return 1;
}
