%{
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "ast/node.h"
#include "ast/ddl.h"

extern FILE *yyin;
int yylex();
int yywrap(void);

void yyerror(std::string str) {}

static bool debug = true;

extern char* lex_str;
extern int64_t int_num;
extern double float_num;

inline void debug_print(std::string info) {
    if (debug) {
        std::cout << info << std::endl;
    }
}

// This is bad because multi sql parsers will use this variable
// TODO Modify this when we need to run in the multi-thread environment
dawn::StmtListNode* ast_root;

%}

%code requires {
    #include "ast/node.h"
    #include "ast/ddl.h"
}

%union{
    int int_val;
    char* str_val;
    dawn::Node* node;
    dawn::DataTypeNode* data_type_node;
    dawn::IdentityNode* id_node;
    dawn::ColumnDefNode* col_def_node;
    dawn::CreateDefNode* create_def_node;
    dawn::CreateDefListNode* create_def_list_node;
    dawn::CreateNode* create_node;
    dawn::DDLNode* ddl_node;
    dawn::StmtListNode* stmt_list_node;
}

%token <str_val> ID
%token <int_val> CHAR INT DECIMAL BOOLEAN
%token
    CREATE DROP INSERT DELETE SELECT TABLE
    INTO VALUES FROM WHERE PRIMARY_KEY
    TRUE FALSE NATURAL JOIN
%token INT_NUM FLOAT_NUM STRING

%left '=' '>' '<' GT_EQ LE_EQ NOT_EQ

%type <stmt_list_node> stmt_list
%type <node> stmt

%type <ddl_node> ddl
%type <create_node> create
%type <node> drop
%type <create_def_list_node> create_def_list
%type <create_def_node> create_def
%type <id_node> col_name
%type <col_def_node> column_def


%type <node> dml
%type <node> insert
%type <node> delete
%type <node> select

%type <data_type_node> data_type
%type <id_node> identity
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

sql: stmt_list {
        debug_print("sql: stmt_list");
        ast_root = $1;
    }

stmt_list
    : stmt {
        debug_print("stmt_list: stmt");
        $$ = new dawn::StmtListNode();
        $$->add_child($1);
    }
    | stmt_list ';' stmt {
        debug_print("stmt_list: stmt_list ';' stmt");
        $1->add_child($3);
        $$ = $1;
    }

stmt: ddl {
        debug_print("sql: ddl");
        $$ = $1;
    }
    | dml {debug_print("sql: dml");}
    | empty_stmt {
        debug_print("sql: empty_stmt");
        $$ = new dawn::StmtNode(dawn::NodeType::kEmptyStmt);
    }

empty_stmt: {debug_print("empty_stmt");}

ddl: create {
        debug_print("ddl: create");
        $$ = new dawn::DDLNode(dawn::DDLType::kCreateTable);
        $$->add_child($1);
    }
    | drop {debug_print("ddl: drop");}

dml: insert {debug_print("dml: insert");}
    | delete {debug_print("dml: delete");}
    | select {debug_print("dml: select");}

create
    : CREATE TABLE identity '(' create_def_list ')' {
        debug_print("create: CREATE TABLE identity '(' create_def_list ')'");
        $$ = new dawn::CreateNode($3, $5);
    }

create_def_list
    : create_def {
        debug_print("create_def_list: create_def");
        $$ = new dawn::CreateDefListNode();
        $$->add_child($1);
    }
    | create_def_list ',' create_def {
        debug_print("create_def_list: create_def_list ',' create_def");
        $1->add_child($3);
        $$ = $1;
    }

create_def
    : col_name column_def {
        debug_print("create_def: col_name column_def");
        $$ = new dawn::CreateDefNode($1, $2);
    }
    | PRIMARY_KEY '(' identity ')' {
        debug_print("create_def: PRIMARY_KEY '(' identity ')'");
        $$ = new dawn::CreateDefNode($3);
    }

col_name
    : identity {
        debug_print("col_name: identity");
        $$ = $1;
    }

column_def
    : data_type {
        debug_print("column_def: data_type");
        $$ = new dawn::ColumnDefNode($1);
    }

data_type
    : CHAR '(' INT_NUM ')' {
        debug_print("data_type: CHAR '(' NUMBER ')'");
        $$ = new dawn::DataTypeNode(dawn::TypeId::kChar, int_num);
    }
    | INT {
        debug_print("data_type: INT");
        $$ = new dawn::DataTypeNode(dawn::TypeId::kInteger);
    }
    | DECIMAL {
        debug_print("data_type: DECIMAL");
        $$ = new dawn::DataTypeNode(dawn::TypeId::kDecimal);
    }
    | BOOLEAN {
        debug_print("data_type: BOOLEAN");
        $$ = new dawn::DataTypeNode(dawn::TypeId::kBoolean);
    }

identity
    : ID {
        debug_print("identity: ID");
        $$ = new dawn::IdentityNode(lex_str);
        delete[] lex_str;
    }

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
    | STRING {
        debug_print("constant: STRING");
        delete[] lex_str;
    }
    | TRUE {
        debug_print("constant: TRUE");
    }
    | FALSE {
        debug_print("constant: FALSE");
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
