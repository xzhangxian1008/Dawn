%include {
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <string.h>

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

%left PLUS MINUS STAR DIVIDE EQUAL GREATER LESS.
%left GT_EQ LE_EQ NOT_EQ OR AND NOT.

%syntax_error {
    debug_print("Error Happends");
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
%type insert {dawn::InsertNode*}
%type value_list {dawn::ValueListNode*}
%type value {dawn::ValueNode*}
%type literal {dawn::LiteralNode*}
%type where_condition {dawn::WhereCondNode*}
%type select {dawn::SelectNode*}
%type select_expr_list {dawn::SelectExprListNode*}
%type table_references {dawn::TableRefsNode*}
%type select_expr {dawn::SelectExprNode*}
%type table_reference {dawn::TableRefNode*}
%type table_factor {dawn::TableFactorNode*}
%type tb_name {dawn::TableNameNode*}
%type expr {dawn::ExprNode*}
%type boolean_primary {dawn::BooleanPrimaryNode*}
%type comparison_operator {dawn::ComparisonOprNode*}
%type predicate {dawn::PredicateNode*}
%type bit_expr {dawn::BitExprNode*}
%type simple_expr {dawn::SimpleExprNode*}
%type delete {dawn::DeleteNode*}

//////////////////////////////// Rules Begin ////////////////////////////////

sql ::= stmt_list(A). {
    debug_print("sql: stmt_list");
    *ast_root = std::move(*A);
    delete A;
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
    debug_print("stmt: ddl");
    A = B;
}

stmt(A) ::= dml(B). {
    debug_print("stmt: dml");
    A = B;
}

stmt(A) ::= empty_stmt. {
    debug_print("stmt: empty_stmt");
    A = new dawn::StmtNode(dawn::NodeType::kEmptyStmt);
}

empty_stmt ::= . {debug_print("empty_stmt");}

ddl(A) ::= create(B). {
    debug_print("ddl: create");
    // A = new dawn::DDLNode(dawn::DDLType::kCreateTable);
    // A->add_child(B);
    A = B;
}

ddl(A) ::= drop(B). {
    debug_print("ddl: drop");
    // A = new dawn::DDLNode(dawn::DDLType::kDropTable);
    // A->add_child(B);
    A = B;
}

dml(A) ::= insert(B). {
    debug_print("dml: insert");
    // A = new dawn::DMLNode(dawn::DMLType::kInsert);
    // A->add_child(B);
    A = B;
}

dml(A) ::= delete(B). {
    debug_print("dml: delete");
    A = B;
}

dml(A) ::= select(B). {
    debug_print("dml: select");
    // A = new dawn::DMLNode(dawn::DMLType::kSelect);
    // A->add_child(B);
    A = B;
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

insert(A) ::= INSERT INTO identifier(B) VALUES LEFT_PARENTHESES value_list(C) RIGHT_PARENTHESES. {
    debug_print("insert: INSERT INTO identifier VALUES '(' value_list ')'");
    A = new dawn::InsertNode(B, C);
}

value_list(A) ::= value(B). {
    debug_print("value_list: value");
    dawn::ValueListNode* node = new dawn::ValueListNode();
    node->add_child(B);
    A = node;
}

value_list(A) ::= value_list(B) COMMA value(C). {
    debug_print("value_list: value_list ',' value");
    B->add_child(C);
    A = B;
}

value(A) ::= literal(B). {
    debug_print("value: literal");
    A = new dawn::ValueNode(B);
}

delete(A) ::= DELETE FROM identifier(B) WHERE where_condition(C). {
    debug_print("delete: DELETE FROM identifier WHERE where_condition");
    
    // The following codes are expedient for tackling the memory leak
    A = new dawn::DeleteNode();
    A->add_child(B);
    A->add_child(C);
}

select(A) ::= SELECT select_expr_list(B) FROM table_references(C) WHERE where_condition(D). {
    debug_print("select: SELECT select_expr FROM table_references WHERE where_condition");
    A = new dawn::SelectNode();
    A->set_select_expr_list(B);
    A->set_table_refs(C);
    A->set_where_cond(D);
}

select_expr_list(A) ::= select_expr(B). {
    debug_print("select_exprs: select_expr");
    A = new dawn::SelectExprListNode();
    if (B->get_type() == dawn::SelectExprNode::SelectExprType::kStar)
        A->set_star(true);
    else
        A->add_child(B);
}

select_expr_list(A) ::= select_expr_list(B) COMMA select_expr(C). {
    debug_print("select_exprs: select_expr_list ',' select_expr");
    if (B->is_star()) {
        // Do nothing if there is a '*' in the expression list
    } else {
        B->add_child(C);
        A = B;
    }
}

select_expr(A) ::= col_name(B). {
    debug_print("select_expr: col_name");
    A = new dawn::SelectExprNode(dawn::SelectExprNode::SelectExprType::kColName);
    A->add_child(B);
}

select_expr(A) ::= STAR. {
    debug_print("select_expr: *");
    A = new dawn::SelectExprNode(dawn::SelectExprNode::SelectExprType::kStar);
}

// We only support the single table so far.
table_references(A) ::= table_reference(B). {
    debug_print("table_references: table_reference");
    A = new dawn::TableRefsNode();
    A->add_child(B);
}

table_references ::= table_references COMMA table_reference. {
    debug_print("table_references: table_references ',' table_reference");
    // We do not support multi tables so far
    // TODO: Cartesian product
}

// Only support single table so far.
table_reference(A) ::= table_factor(B). {
    debug_print("table_reference: table_factor");
    A = new dawn::TableRefNode();
    A->add_child(B);
}

table_reference ::= joined_table. {
    debug_print("table_reference: joined_table");
}

table_factor(A) ::= tb_name(B). {
    debug_print("table_factor: tb_name");
    A = new dawn::TableFactorNode();
    A->add_child(B);
}

joined_table ::= table_reference NATURAL JOIN table_factor. {
    debug_print("joined_table: table_reference NATURAL JOIN table_factor");
}

tb_name(A) ::= identifier(B). {
    debug_print("tb_name: identifier");
    A = B;
}

where_condition(A) ::= expr(B). {
    debug_print("where_condition: expr");
    A = B;
}

expr(A) ::= expr(B) AND expr(C). {
    debug_print("expr: expr AND expr");
    A = new dawn::ExprNode(dawn::ExprNode::ExprType::kAND);
    A->add_child(B);
    A->add_child(C);
}

expr(A) ::= expr(B) OR expr(C). {
    debug_print("expr: expr OR expr");
    A = new dawn::ExprNode(dawn::ExprNode::ExprType::kOR);
    A->add_child(B);
    A->add_child(C);
}

expr(A) ::= NOT expr(B). {
    debug_print("expr: NOT expr");
    A = new dawn::ExprNode(dawn::ExprNode::ExprType::kNOT);
    A->add_child(B);
}

expr(A) ::= boolean_primary(B). {
    debug_print("expr: boolean_primary");
    A = new dawn::ExprNode(dawn::ExprNode::ExprType::kBooleanPrimary);
    A->add_child(B);
}

boolean_primary(A) ::= boolean_primary(B) comparison_operator(C) predicate(D). {
    debug_print("boolean_primary: boolean_primary comparison_operator predicate");
    A = new dawn::BooleanPrimaryNode();
    A->set_recursive(true);
    A->set_comparison_opr(true);
    A->add_child(B);
    A->add_child(C);
    A->add_child(D);
}

boolean_primary(A) ::= predicate(B). {
    debug_print("boolean_primary: predicate");
    A = new dawn::BooleanPrimaryNode();
    A->add_child(B);
}

comparison_operator(A) ::= EQUAL. {
    debug_print("comparison_operator: =");
    A = new dawn::ComparisonOprNode(dawn::ComparisonOprNode::ComparisonOprType::kEqual);
}

comparison_operator(A) ::= GREATER_SIGN. {
    debug_print("comparison_operator: >");
    A = new dawn::ComparisonOprNode(dawn::ComparisonOprNode::ComparisonOprType::kGreater);
}

comparison_operator(A) ::= LESS_SIGN. {
    debug_print("comparison_operator: <");
    A = new dawn::ComparisonOprNode(dawn::ComparisonOprNode::ComparisonOprType::kLess);
}

comparison_operator(A) ::= GT_EQ. {
    debug_print("comparison_operator: GT_EQ");
    A = new dawn::ComparisonOprNode(dawn::ComparisonOprNode::ComparisonOprType::kGreaterEq);
}

comparison_operator(A) ::= LE_EQ. {
    debug_print("comparison_operator: LE_EQ");
    A = new dawn::ComparisonOprNode(dawn::ComparisonOprNode::ComparisonOprType::kLessEq);
}

comparison_operator(A) ::= NOT_EQ. {
    debug_print("comparison_operator: NOT_EQ");
    A = new dawn::ComparisonOprNode(dawn::ComparisonOprNode::ComparisonOprType::kNotEqual);
}

predicate(A) ::= bit_expr(B). {
    debug_print("predicate: bit_expr");
    dawn::PredicateNode* predicate = new dawn::PredicateNode();
    predicate->add_child(B);
    A = predicate;
}

bit_expr(A) ::= bit_expr(B) PLUS bit_expr(C). {
    debug_print("bit_expr: bit_expr '+' bit_expr");
    dawn::BitExprNode* bit_expr = new dawn::BitExprNode(dawn::BitExprNode::BitExprType::kPlus);
    bit_expr->add_child(B);
    bit_expr->add_child(C);
    A = bit_expr;
}

bit_expr(A) ::= bit_expr(B) MINUS bit_expr(C). {
    debug_print("bit_expr: bit_expr '-' bit_expr");
    dawn::BitExprNode* bit_expr = new dawn::BitExprNode(dawn::BitExprNode::BitExprType::kMinus);
    bit_expr->add_child(B);
    bit_expr->add_child(C);
    A = bit_expr;
}

bit_expr(A) ::= bit_expr(B) STAR bit_expr(C). {
    debug_print("bit_expr: bit_expr '*' bit_expr");
    dawn::BitExprNode* bit_expr = new dawn::BitExprNode(dawn::BitExprNode::BitExprType::kMultiply);
    bit_expr->add_child(B);
    bit_expr->add_child(C);
    A = bit_expr;
}

bit_expr(A) ::= bit_expr(B) DIVIDE bit_expr(C). {
    debug_print("bit_expr: bit_expr '/' bit_expr");
    dawn::BitExprNode* bit_expr = new dawn::BitExprNode(dawn::BitExprNode::BitExprType::kDivide);
    bit_expr->add_child(B);
    bit_expr->add_child(C);
    A = bit_expr;
}

bit_expr(A) ::= simple_expr(B). {
    debug_print("bit_expr: simple_expr");
    dawn::BitExprNode* bit_expr = new dawn::BitExprNode(dawn::BitExprNode::BitExprType::kSimpleExpr);
    bit_expr->add_child(B);
    A = bit_expr;
}

simple_expr(A) ::= literal(B). {
    debug_print("simple_expr: literal");
    A = new dawn::SimpleExprNode(dawn::SimpleExprNode::SimpleExprType::kLiteral);
    A->add_child(B);
}

simple_expr(A) ::= identifier(B). {
    debug_print("simple_expr: identifier");
    A = new dawn::SimpleExprNode(dawn::SimpleExprNode::SimpleExprType::kIdentifier);
    A->add_child(B);
}

literal(A) ::= INT_NUM(B). {
    debug_print("literal: INT_NUM");
    A = new dawn::LiteralNode();
    A->set_integer(B.val_.integer_);
}

literal(A) ::= DECIMAL_NUM(B). {
    debug_print("literal: FLOAT_NUM");
    A = new dawn::LiteralNode();
    A->set_decimal(B.val_.decimal_);
}

literal(A) ::= STRING(B). {
    debug_print("literal: STRING");
    A = new dawn::LiteralNode();
    A->set_str(B.val_.varchar_, strlen(B.val_.varchar_));
}

literal(A) ::= TRUE. {
    debug_print("literal: TRUE");
    A = new dawn::LiteralNode();
    A->set_boolean(true);
}

literal(A) ::= FALSE. {
    debug_print("literal: FALSE");
    A = new dawn::LiteralNode();
    A->set_boolean(false);
}
