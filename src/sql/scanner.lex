%{
#include <string>
#include <cstring>

char* str_id;
int64_t integer;
double float;
%}

digit   [0-9]
letter  [A-Za-z]
id      ({letter}|_)({letter}|{digit}|_)*
integer [1-9]{digit}*|0
float   {integer}\.{integer}
%%

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
{id}    {
            str_id = new char[yyleng+1];
            std::memcpy(str_id, yytext, yyleng);
            str_id[yyleng] = 0;
            return ID;
        }

%%

int yywrap(void) {
    return 1;
}
