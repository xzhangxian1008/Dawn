/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 36 "parser.yy" /* yacc.c:1909  */

    #include "ast/node.h"
    #include "ast/ddl.h"
    #include "ast/dml.h"

#line 50 "y.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ID = 258,
    CHAR = 259,
    INT = 260,
    DECIMAL = 261,
    BOOLEAN = 262,
    CREATE = 263,
    DROP = 264,
    INSERT = 265,
    DELETE = 266,
    SELECT = 267,
    TABLE = 268,
    INTO = 269,
    VALUES = 270,
    FROM = 271,
    WHERE = 272,
    PRIMARY_KEY = 273,
    TRUE = 274,
    FALSE = 275,
    NATURAL = 276,
    JOIN = 277,
    INT_NUM = 278,
    FLOAT_NUM = 279,
    STRING = 280,
    GT_EQ = 281,
    LE_EQ = 282,
    NOT_EQ = 283,
    OR = 284,
    AND = 285,
    NOT = 286
  };
#endif
/* Tokens.  */
#define ID 258
#define CHAR 259
#define INT 260
#define DECIMAL 261
#define BOOLEAN 262
#define CREATE 263
#define DROP 264
#define INSERT 265
#define DELETE 266
#define SELECT 267
#define TABLE 268
#define INTO 269
#define VALUES 270
#define FROM 271
#define WHERE 272
#define PRIMARY_KEY 273
#define TRUE 274
#define FALSE 275
#define NATURAL 276
#define JOIN 277
#define INT_NUM 278
#define FLOAT_NUM 279
#define STRING 280
#define GT_EQ 281
#define LE_EQ 282
#define NOT_EQ 283
#define OR 284
#define AND 285
#define NOT 286

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 42 "parser.yy" /* yacc.c:1909  */

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
    dawn::DMLNode* dml_node;
    dawn::StmtListNode* stmt_list_node;
    dawn::DropNode* drop_node;
    dawn::LiteralNode* literal_node;
    dawn::ValueListNode* value_list_node;
    dawn::ValueNode* value_node;
    dawn::InsertNode* insert_node;
    dawn::ExprNode* expr_node;
    dawn::BooleanPrimaryNode* boolean_primary_node;
    dawn::ComparisonOprNode* comparison_opr_node;
    dawn::PredicateNode* predicate_node;
    dawn::BitExprNode* bit_expr_node;
    dawn::SimpleExprNode* simple_expr_node;
    dawn::WhereCondNode* where_condition_node;
    dawn::SelectNode* select_node;
    dawn::SelectExprListNode* select_expr_list_node;
    dawn::SelectExprNode* select_expr_node;
    dawn::TableRefsNode* table_references_node;
    dawn::TableRefNode* table_reference_node;
    dawn::TableFactorNode* table_factor_node;
    dawn::TableNameNode* tb_name_node;

#line 158 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
