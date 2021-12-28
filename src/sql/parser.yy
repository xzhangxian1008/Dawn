%{
#include <stdio.h>
#include <iostream>

extern FILE *yyin;
int yylex();
int yywrap(void);

void yyerror(std::string str) {}

static bool debug = true;

extern char* str_id;
extern int64_t integer;
extern double float;

void debug_print(std::string info) {
    if (debug) {
        std::cout << info << std::endl;
    }
}
%}

%union{
    int int_val;
    char* str_val;
    dawn::ExecutorAbstract* node;
}

%token <int_val> NUMBER
%token <str_val> ID
%token <int_val> CHAR INT DECIMAL BOOLEAN
%token
    CREATE DROP INSERT DELETE SELECT TABLE
    INTO VALUES FROM WHERE PRIMARY_KEY
    TRUE FALSE

%type <node> ddl
%type <node> dml
%type <node> create
%type <node> drop
%type <node> insert
%type <node> delete
%type <node> select

%start sql

%%

sql: stmt_list {debug_print("sql: stmt_list");}

stmt_list
    : stmt {debug_print("stmt_list: stmt");}
    : stmt_list ';' stmt {debug_print("stmt_list: stmt_list ';' stmt");}

stmt: ddl {debug_print("sql: ddl");}
    | dml {debug_print("sql: dml");}

ddl: create {debug_print("ddl: create");}
    | drop {debug_print("ddl: drop");}

dml: insert {debug_print("dml: insert");}
    | delete {debug_print("dml: delete");}
    | select {debug_print("dml: select");}

create
    : CREATE TABLE ID '(' create_def_list ')' {
        debug_print("create: CREATE TABLE ID '(' create_def_list ')'");
    }

create_def_list
    : create_def {debug_print("create_def_list: create_def");}
    | create_def_list ',' create_def {debug_print("create_def_list: create_def_list ',' create_def");}

create_def
    : col_name column_def {
        debug_print("create_def: col_name column_def");
    }
    | PRIMARY_KEY '(' identity ')' {
        debug_print("create_def: PRIMARY_KEY '(' identity ')'");
    }

col_name: identity {debug_print("col_name: identity");}

column_def: data_type {debug_print("column_def: data_type");}

data_type
    : CHAR '(' NUMBER ')' {
        debug_print("data_type: CHAR '(' NUMBER ')'");
    }
    | INT {debug_print("data_type: INT");}
    | DECIMAL {debug_print("data_type: DECIMAL");}
    | BOOLEAN {debug_print("data_type: BOOLEAN");}

identity: ID {debug_print("identity: ID");}

drop: DROP TABLE identity {debug_print("drop: DROP TABLE identity");}

insert: INSERT INTO identity VALUES '(' value_list ')' {debug_print("insert: INSERT INTO identity VALUES '(' value_list ')'");}

value_list
    : value {debug_print("value_list: value");}
    | value_list ',' value {debug_print("value_list: value_list ',' value");}

delete
    : DELETE FROM identity WHERE where_condition {
        debug_print("delete: DELETE FROM identity WHERE where_condition");
    }

expr_list:
    : expr {debug_print("expr_list: expr");}
    | expr_list ',' expr {debug_print("expr_list: expr_list ',' expr");}

expr
    : expr '=' expr {debug_print("expr: expr '=' expr");}
    | expr '>' expr {debug_print("expr: expr '>' expr");}
    | expr '<' expr {debug_print("expr: expr '<' expr");}
    | expr GT_EQ expr {debug_print("expr: expr GT_EQ expr");}
    | expr LE_EQ expr {debug_print("expr: expr LE_EQ expr");}
    | expr NOT_EQ expr {debug_print("expr: expr NOT_EQ expr");}
    | constant {debug_print("expr: constant");}
    | identity {debug_print("expr: identity");}

constant
    : NUMBER {
        debug_print("constant: NUMBER");
    }

select
    : SELECT select_expr FROM table_references WHERE where_condition {
        debug_print("select: SELECT select_expr FROM table_references WHERE where_condition");
    }

where_condition
    : expr_list {debug_print("where_condition: expr_list");}
