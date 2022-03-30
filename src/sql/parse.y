%include {
#include <stdio.h>
#include <iostream>
#include <assert.h>

#include "ast/node.h"
#include "ast/ddl.h"
#include "ast/dml.h"
#include "util/config.h"
#include "sql/lex.h"

static bool debug = true;

inline void debug_print(std::string info) {
    if (debug) {
        std::cout << info << std::endl;
    }
}
}

%token_type {dawn::Token}
%token_prefix TOKEN_
%extra_argument {dawn::StmtListNode* ast_root}
%name dawn_parse

%left PLUS MINUS MULTIPLY DIVIDE EQUAL GREATER LESS.
%left GT_EQ LE_EQ NOT_EQ OR AND NOT.

%syntax_error {
    ast_root->set_error();
}

////////////////////// Non-terminal Type Definition //////////////////////

%type stmt_list {dawn::StmtListNode*}
%type stmt {dawn::StmtNode*}
%type ddl {dawn::DDLNode*}
%type dml {dawn::DMLNode*}
%type create {dawn::CreateNode*}
%type drop {dawn::DropNode*}
%type identifier {dawn::IdentifierNode*}
%type create_def_list {dawn::CreateDefListNode*}
%type create_def {dawn::CreateDefNode*}
%type col_name {dawn::IdentifierNode*}
%type column_def {dawn::ColumnDefNode*}
%type data_type {dawn::DataTypeNode*}

//////////////////////////////// Rules Begin ////////////////////////////////

sql ::= stmt_list(A). {
    debug_print("sql: stmt_list");
    ast_root = A;
}

sql ::= INSERT INTO VALUES DELETE FROM WHERE SELECT SINGLE_QUOTES DOUBLE_QUOTES STRING TRUE FALSE LESS_SIGN GREATER_SIGN DECIMAL_NUM. {
    // tmp
}

stmt_list(A) ::= stmt(B). {
    debug_print("stmt_list: stmt");
    A = new dawn::StmtListNode();
    A->add_child(B);
}

stmt_list(A) ::= stmt_list(B) SEMICOLON stmt(C). {
    debug_print("stmt_list: stmt_list ';' stmt");
    B->add_child(C);
    A = B;
}

stmt(A) ::= ddl(B). {
    debug_print("sql: ddl");
    A = B;
}

stmt(A) ::= empty_stmt. {
    debug_print("sql: empty_stmt");
    A = new dawn::StmtNode(dawn::NodeType::kEmptyStmt);
}

empty_stmt ::= . {debug_print("empty_stmt");}

ddl(A) ::= create(B). {
    debug_print("ddl: create");
    A = new dawn::DDLNode(dawn::DDLType::kCreateTable);
    A->add_child(B);
}

ddl(A) ::= drop(B). {
    debug_print("ddl: drop");
    A = new dawn::DDLNode(dawn::DDLType::kDropTable);
    A->add_child(B);
}

create(A) ::= CREATE TABLE identifier(B) LEFT_PARENTHESES create_def_list(C) RIGHT_PARENTHESES. {
    debug_print("create: CREATE TABLE identifier '(' create_def_list ')'");
    A = new dawn::CreateNode(B, C);
}

create_def_list(A) ::= create_def(B). {
    debug_print("create_def_list: create_def");
    A = new dawn::CreateDefListNode();
    A->add_child(B);
}

create_def_list(A) ::= create_def_list(B) COMMA create_def(C). {
    debug_print("create_def_list: create_def_list ',' create_def");
    B->add_child(C);
    A = B;
}

create_def(A) ::= col_name(B) column_def(C). {
    debug_print("create_def: col_name column_def");
    A = new dawn::CreateDefNode(B, C);
}

create_def(A) ::= PRIMARY_KEY LEFT_PARENTHESES identifier(B) RIGHT_PARENTHESES. {
    debug_print("create_def: PRIMARY_KEY '(' identifier ')'");
    A = new dawn::CreateDefNode(B);
}

col_name(A) ::= identifier(B). {
    debug_print("col_name: identifier");
    A = B;
}

column_def(A) ::= data_type(B). {
    debug_print("column_def: data_type");
    A = new dawn::ColumnDefNode(B);
}

data_type(A) ::= CHAR LEFT_PARENTHESES INT_NUM(B) RIGHT_PARENTHESES. {
    debug_print("data_type: CHAR '(' NUMBER ')'");
    A = new dawn::DataTypeNode(dawn::TypeId::kChar, B.val_.integer_);
}

data_type(A) ::= INT. {
    debug_print("data_type: INT");
    A = new dawn::DataTypeNode(dawn::TypeId::kInteger);
}

data_type(A) ::= DECIMAL. {
    debug_print("data_type: DECIMAL");
    A = new dawn::DataTypeNode(dawn::TypeId::kDecimal);
}

data_type(A) ::= BOOLEAN. {
    debug_print("data_type: BOOLEAN");
    A = new dawn::DataTypeNode(dawn::TypeId::kBoolean);
}

identifier(A) ::= ID(B). {
    debug_print("identifier: ID");
    A = new dawn::IdentifierNode(B.val_.varchar_);
    delete[] B.val_.varchar_;
}

drop(A) ::= DROP TABLE identifier(B). {
    debug_print("drop: DROP TABLE identifier");
    A = new dawn::DropNode(B);
}
