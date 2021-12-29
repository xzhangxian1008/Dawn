%{
#include <stdio.h>
#include <iostream>

extern FILE *yyin;
int yylex();
int yywrap(void);

void yyerror(std::string str) {}

static bool debug = true;

extern char* str_id;
extern int64_t int_num;
extern double float_num;

void debug_print(std::string info) {
    if (debug) {
        std::cout << info << std::endl;
    }
}

%}

%union{
    int int_val;
    char* str_val;
    int node;
}

%token <str_val> ID
%token <int_val> CHAR INT DECIMAL BOOLEAN
%token
    CREATE DROP INSERT DELETE SELECT TABLE
    INTO VALUES FROM WHERE PRIMARY_KEY
    TRUE FALSE NATURAL JOIN
%token INT_NUM FLOAT_NUM STRING

%left '=' '>' '<' GT_EQ LE_EQ NOT_EQ

%type <node> stmt_list
%type <node> stmt

%type <node> ddl
%type <node> create
%type <node> drop
%type <node> create_def_list
%type <node> create_def
%type <node> col_name
%type <node> column_def


%type <node> dml
%type <node> insert
%type <node> delete
%type <node> select

%type <node> data_type
%type <node> identity
%type <node> expr_list
%type <node> expr
%type <node> constant
%type <node> where_condition
%type <node> value_list
%type <node> value
%type <node> table_references
%type <node> table_reference
%type <node> table_factor
%type <node> joined_table
%type <node> tbl_name

%start sql

%%

sql: stmt_list {debug_print("sql: stmt_list");}

stmt_list
    : stmt {debug_print("stmt_list: stmt");}
    | stmt_list ';' stmt {debug_print("stmt_list: stmt_list ';' stmt");}

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
    : CHAR '(' INT_NUM ')' {
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

value: constant {debug_print("value: constant");}

delete
    : DELETE FROM identity WHERE where_condition {
        debug_print("delete: DELETE FROM identity WHERE where_condition");
    }

expr_list
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
    : INT_NUM {
        debug_print("constant: INT_NUM");
    }
    | FLOAT_NUM {
        debug_print("constant: FLOAT_NUM");
    }

select
    : SELECT select_expr_list FROM table_references WHERE where_condition {
        debug_print("select: SELECT select_expr FROM table_references WHERE where_condition");
    }

select_expr_list
    : select_expr {debug_print("select_exprs: select_expr");}
    | select_expr_list ',' select_expr {debug_print("select_exprs: select_expr_list ',' select_expr");}

select_expr
    : col_name {debug_print("select_expr: col_name");}

table_references
    : table_reference {debug_print("table_references: table_reference");}
    | table_references ',' table_reference {
        debug_print("table_references: table_references ',' table_reference");
    }

table_reference
    : table_factor {debug_print("table_reference: table_factor");}
    | joined_table {debug_print("table_reference: joined_table");}

table_factor
    : tbl_name {debug_print("table_factor: tbl_name");}

joined_table
    : table_reference NATURAL JOIN table_factor {
        debug_print("joined_table: table_reference NATURAL JOIN table_factor");
    }

tbl_name
    : identity {debug_print("tbl_name: identity");}

where_condition
    : expr_list {debug_print("where_condition: expr_list");}
