%{
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "ast/node.h"
#include "ast/ddl.h"
#include "ast/dml.h"
#include "util/config.h"

extern FILE *yyin;
int yylex();
int yywrap(void);

void yyerror(std::string str) {}

static bool debug = true;

extern int yyleng;
extern dawn::varchar_t lex_str;
extern dawn::integer_t int_num;
extern dawn::decimal_t float_num;

inline void debug_print(std::string info) {
    if (debug) {
        std::cout << info << std::endl;
    }
}

// This is bad because multi sql parsers will use this variable
// TODO: Modify this when we need to run in the multi-thread environment
dawn::StmtListNode* ast_root;

%}

%code requires {
    #include "ast/node.h"
    #include "ast/ddl.h"
    #include "ast/dml.h"
}

%union{
    int int_val;
    char* str_val;
    dawn::Node* node;
    dawn::DataTypeNode* data_type_node;
    dawn::IdentifierNode* id_node;
    dawn::ColumnDefNode* col_def_node;
    dawn::CreateDefNode* create_def_node;
    dawn::CreateDefListNode* create_def_list_node;
    dawn::CreateNode* create_node;
    dawn::DDLNode* ddl_node;
    dawn::StmtListNode* stmt_list_node;
    dawn::DropNode* drop_node;
    dawn::LiteralNode* literal_node;
    dawn::ValueListNode* value_list_node;
    dawn::ValueNode* value_node;
    dawn::InsertNode* insert_node;
}

%token <str_val> ID
%token <int_val> CHAR INT DECIMAL BOOLEAN
%token
    CREATE DROP INSERT DELETE SELECT TABLE
    INTO VALUES FROM WHERE PRIMARY_KEY
    TRUE FALSE NATURAL JOIN
%token INT_NUM FLOAT_NUM STRING

%left '+' '-' '*' '/' '=' '>' '<' GT_EQ LE_EQ NOT_EQ OR AND

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
%type <insert_node> insert
%type <node> delete
%type <node> select

%type <data_type_node> data_type
%type <id_node> identifier
%type <node> expr
%type <literal_node> literal
%type <node> where_condition
%type <value_list_node> value_list
%type <value_node> value
%type <node> table_references
%type <node> table_reference
%type <node> table_factor
%type <node> joined_table
%type <node> tb_name

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
    | dml {
        debug_print("sql: dml");
        $$ = $1;
    }
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
    | drop {
        debug_print("ddl: drop");
        $$ = new dawn::DDLNode(dawn::DDLType::kDropTable);
        $$->add_child($1);
    }

dml
    : insert {
        debug_print("dml: insert");
        $$ = new dawn::DMLNode(dawn::DMLType::kInsert);
        $$->add_child($1);
    }
    | delete {debug_print("dml: delete");}
    | select {debug_print("dml: select");}

create
    : CREATE TABLE identifier '(' create_def_list ')' {
        debug_print("create: CREATE TABLE identifier '(' create_def_list ')'");
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
    | PRIMARY_KEY '(' identifier ')' {
        debug_print("create_def: PRIMARY_KEY '(' identifier ')'");
        $$ = new dawn::CreateDefNode($3);
    }

col_name
    : identifier {
        debug_print("col_name: identifier");
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

identifier
    : ID {
        debug_print("identifier: ID");
        $$ = new dawn::IdentifierNode(lex_str);
        delete[] lex_str;
    }

drop
    : DROP TABLE identifier {
        debug_print("drop: DROP TABLE identifier");
        $$ = new dawn::DropNode($3);
    }

insert
    : INSERT INTO identifier VALUES '(' value_list ')' {
        debug_print("insert: INSERT INTO identifier VALUES '(' value_list ')'");
        $$ = new dawn::InsertNode($3, $6);
    }

value_list
    : value {
        debug_print("value_list: value");
        dawn::ValueListNode* node = new dawn::ValueListNode();
        node->add_child($1);
        $$ = node;
    }
    | value_list ',' value {
        debug_print("value_list: value_list ',' value");
        $1->add_child($3);
        $$ = $1;
    }

value
    : literal {
        debug_print("value: literal");
        $$ = new dawn::ValueNode($1);
    }

delete
    : DELETE FROM identifier WHERE where_condition {
        debug_print("delete: DELETE FROM identifier WHERE where_condition");
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
    | '*' {
        debug_print("select_expr: *");
    }

// We only support the single table so far.
table_references
    : table_reference {
        debug_print("table_references: table_reference");
    }
    | table_references ',' table_reference {
        debug_print("table_references: table_references ',' table_reference");
        // TODO: Cartesian product
    }

// Only support single table so far.
table_reference
    : table_factor {debug_print("table_reference: table_factor");}
    | joined_table {debug_print("table_reference: joined_table");}

table_factor
    : tb_name {debug_print("table_factor: tb_name");}

joined_table
    : table_reference NATURAL JOIN table_factor {
        debug_print("joined_table: table_reference NATURAL JOIN table_factor");
    }

tb_name
    : identifier {debug_print("tb_name: identifier");}

where_condition
    : expr {debug_print("where_condition: expr");}

expr
    : expr AND expr {debug_print("expr: expr AND expr");}
    | expr OR expr {debug_print("expr: expr OR expr");}
    | boolean_primary {debug_print("expr: boolean_primary");}

boolean_primary
    : boolean_primary comparison_operator predicate {
        debug_print("boolean_primary: boolean_primary comparison_operator predicate");
    }
    | predicate {debug_print("boolean_primary: predicate");}

comparison_operator
    : '=' {debug_print("comparison_operator: =");}
    | '>' {debug_print("comparison_operator: >");}
    | GT_EQ {debug_print("comparison_operator: GT_EQ");}
    | LE_EQ {debug_print("comparison_operator: LE_EQ");}
    | NOT_EQ {debug_print("comparison_operator: NOT_EQ");}

predicate
    : bit_expr {debug_print("predicate: bit_expr");}

bit_expr
    : bit_expr '+' bit_expr {debug_print("bit_expr: bit_expr '+' bit_expr");}
    | bit_expr '-' bit_expr {debug_print("bit_expr: bit_expr '-' bit_expr");}
    | bit_expr '*' bit_expr {debug_print("bit_expr: bit_expr '*' bit_expr");}
    | bit_expr '/' bit_expr {debug_print("bit_expr: bit_expr '/' bit_expr");}
    | simple_expr {debug_print("bit_expr: simple_expr");}

simple_expr
    : literal {debug_print("simple_expr: literal");}
    | identifier {debug_print("simple_expr: identifier");}

literal
    : INT_NUM {
        debug_print("literal: INT_NUM");
        $$ = new dawn::LiteralNode();
        $$->set_integer(int_num);
    }
    | FLOAT_NUM {
        debug_print("literal: FLOAT_NUM");
        $$ = new dawn::LiteralNode();
        $$->set_decimal(float_num);
    }
    | STRING {
        debug_print("literal: STRING");
        $$ = new dawn::LiteralNode();
        $$->set_str(lex_str, yyleng-2); // These two "'" should be ignored
    }
    | TRUE {
        debug_print("literal: TRUE");
        $$ = new dawn::LiteralNode();
        $$->set_boolean(true);
    }
    | FALSE {
        debug_print("literal: FALSE");
        $$ = new dawn::LiteralNode();
        $$->set_boolean(false);
    }
