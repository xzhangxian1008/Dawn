#include "gtest/gtest.h"
#include <assert.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>

#include "ast/ddl.h"
#include "ast/dml.h"
#include "manager/db_manager.h"
#include "sql/sql_parse.h"

void* dawn_parseAlloc(void*(*)(size_t));
void dawn_parseFree(void*p, void(*freeProc)(void*));
void dawn_parse(void*, int, dawn::Token, dawn::StmtListNode*);

namespace dawn {

bool create_table(CreateNode* node);
bool drop_table(DropNode* node);
bool insert_data(InsertNode* node);
extern std::unique_ptr<DBManager> db_manager;

/** DO NOT DELETE THE db */
bool manually_create_tb(
    DBManager* db,
    const string_t& tb_name,
    const std::vector<TypeId>& types,
    const std::vector<string_t>& names,
    const std::vector<size_t_>& char_len = std::vector<size_t_>{})
{
    Catalog* catalog = db->get_catalog();
    CatalogTable* catalog_tb = catalog->get_catalog_table();

    // check duplication of table name
    page_id_t page_id = catalog_tb->get_table_id(tb_name);
    if (page_id != -1) {
        // duplicated table name
        return false;
    }

    std::unique_ptr<Schema> schema(create_table_schema(types, names, char_len));

    if (!catalog_tb->create_table(tb_name, *schema)) {
        return false;
    }
    return true;
}

inline std::unique_ptr<Lex> build_lex(const char* file_name) {
    FILE* file = fopen(file_name,"r");
    assert(file);
    return std::make_unique<Lex>(file);
}

class ParserTests : public testing::Test {
public:
    void SetUp() {}

    void TearDown() {
        remove(meta_name);
        remove(db_name);
        remove(log_name);
    }

    string_t tb_name{"books"};
    std::vector<TypeId> col_types{TypeId::kBoolean, TypeId::kInteger};
    std::vector<string_t> col_names{"tb1_col1", "tb1_col2"};

    const char *meta = "test";
    const char *meta_name = "test.mtd";
    const char *db_name = "test.db";
    const char *log_name = "test.log";
};

/**
 * Test List:
 *   1. Check if we can parse "parser_test0" successfully.
 */
TEST_F(ParserTests, ParserTest0) {
    FILE* file = fopen("./test/parser_test0","r");
    ASSERT_NE(file, nullptr);

    Lex lex(file);
    void* parser = dawn_parseAlloc(malloc);
    StmtListNode* ast_root = new StmtListNode;
    Token tk;

    while (lex.next_token(&tk)) {
        dawn_parse(parser, tk.type_, tk, ast_root);
    }

    dawn_parse(parser, 0, tk, ast_root);

    EXPECT_FALSE(ast_root->is_error());
    EXPECT_EQ(ast_root->get_stmt_nodes().size(), 6);
    
    dawn_parseFree(parser, free);
    delete ast_root;
}

/**
 * Test CREATE TABLE
 * 
 * Test List:
 *   1. Create the table successfully and fail to do it when it's duplicated
 *   2. Ensure the tables are stored in the disk and can be read
 *      after we reboot the DB.
 */
TEST_F(ParserTests, ParserTest1) {
    std::string file_path("./test/parser_test1");
    std::string tb_name("books");

    // Test 1
    {
        db_manager = std::make_unique<DBManager>(meta, true);
        ASSERT_TRUE(db_manager->get_status());

        {
            // Creating table should be successful
            std::unique_ptr<Lex> lex = build_lex(file_path.c_str());
            EXPECT_TRUE(sql_execute(*lex).is_success());
        }

        // check if table exists in the db
        Catalog* catalog = db_manager->get_catalog();
        CatalogTable* catalog_tb = catalog->get_catalog_table();
        EXPECT_EQ(catalog_tb->get_table_num(), 1); // So far, we create only one table
        EXPECT_NE(catalog_tb->get_table_id(tb_name), -1); // Check if we can get the table

        {
            // Create the same table again and it should be fail
            std::unique_ptr<Lex> lex = build_lex(file_path.c_str());
            EXPECT_FALSE(sql_execute(*lex).is_success());

            db_manager.reset(nullptr); // Shutdown
        }
    }

    // Test 2
    {
        // Reboot the db
        db_manager = std::make_unique<DBManager>(meta, false);
        ASSERT_TRUE(db_manager->get_status());

        // check if table exists in the db
        Catalog* catalog = db_manager->get_catalog();
        CatalogTable* catalog_tb = catalog->get_catalog_table();
        EXPECT_EQ(catalog_tb->get_table_num(), 1); // So far, we create only one table
        EXPECT_NE(catalog_tb->get_table_id(tb_name), -1); // Check if we can get the table
    }
}

/**
 * Test DROP TABLE
 * 
 * Test List:
 *   1. Check if drop table successfully and we can create it again.
 *      Return true when delete an nonexistent table
 *   2. Drop table and reboot db, this table should no longer be found
 */
TEST_F(ParserTests, ParserTest2) {
    std::string file_path("./test/parser_test2");

    // Test 1
    {
        db_manager = std::make_unique<DBManager>(meta, true);
        ASSERT_TRUE(db_manager->get_status());

        // Ensure we manually create table successful
        Catalog* catalog = db_manager->get_catalog();
        CatalogTable* catalog_tb = catalog->get_catalog_table();
        ASSERT_TRUE(manually_create_tb(db_manager.get(), tb_name, col_types, col_names));
        ASSERT_NE(catalog_tb->get_table_id(tb_name), -1);

        {
            // Drop table
            std::unique_ptr<Lex> lex = build_lex(file_path.c_str());
            EXPECT_TRUE(sql_execute(*lex).is_success());
        }

        // Ensure that we can't find it
        EXPECT_EQ(catalog_tb->get_table_id(tb_name), -1);

        // Drop again and it's success
        EXPECT_TRUE(catalog_tb->delete_table(tb_name));

        // We should be able to create this table again
        EXPECT_TRUE(manually_create_tb(db_manager.get(), tb_name, col_types, col_names));
        EXPECT_NE(catalog_tb->get_table_id(tb_name), -1);

        // Drop again to prepare for Test 2
        EXPECT_TRUE(catalog_tb->delete_table(tb_name));

        db_manager.reset(nullptr); // Shutdown
    }

    // Test 2
    {
        db_manager = std::make_unique<DBManager>(meta, false);
        ASSERT_TRUE(db_manager->get_status());

        Catalog* catalog = db_manager->get_catalog();
        CatalogTable* catalog_tb = catalog->get_catalog_table();

        // This table should no longer be found after reboot
        EXPECT_EQ(catalog_tb->get_table_id(tb_name), -1);
    }
}

/**
 * Test INSERT INTO table
 * 
 * Test List:
 *   1. Insert data into table successfully,
 *      find this data successfully,
 *      reinsert the same data into table unsuccessfully,
 *      reboot db and find this data successfully.
 *   
 */
TEST_F(ParserTests, ParserTest3) {
    std::string file_path("./test/parser_test3");
    string_t tb_name{"books"};
    std::vector<TypeId> col_types{
        TypeId::kInteger, TypeId::kChar, TypeId::kDecimal, TypeId::kBoolean};
    std::vector<string_t> col_names{"tb1_col1", "tb1_col2", "tb1_col3", "tb1_col4"};
    std::vector<size_t_> char_len{4};

    // Test 1
    {
        {
            db_manager = std::make_unique<DBManager>(meta, true);
            ASSERT_TRUE(db_manager->get_status());

            // Ensure we create table manually successfully 
            Catalog* catalog = db_manager->get_catalog();
            CatalogTable* catalog_tb = catalog->get_catalog_table();
            ASSERT_TRUE(
                manually_create_tb(db_manager.get(), tb_name, col_types, col_names, char_len)
            );
            ASSERT_NE(catalog_tb->get_table_id(tb_name), -1);

            {
                // Insert data into table, ought to be successfully
                std::unique_ptr<Lex> lex = build_lex(file_path.c_str());
                ASSERT_TRUE(sql_execute(*lex).is_success());
            }

            // Find this data.
            {
                // Key value is set manually corresponding to the file parser_test3, default key value index is 0
                Value key_value(1);
                TableMetaData* table_meta = catalog_tb->get_table_meta_data(tb_name);
                const Schema* schema = table_meta->get_table_schema();
                Table* table = table_meta->get_table();
                ASSERT_NE(table, nullptr);
                Tuple tuple;
                ASSERT_TRUE(table->get_tuple(key_value, &tuple, *schema)); // Find this data

                // These values are set manually corresponding to the file parser_test3
                std::vector<Value> values{Value(1), Value("1234"), Value(23.33), Value(true)};

                // Check
                size_t size = values.size();
                for (size_t i = 0; i < size; i++) {
                    EXPECT_TRUE(values[i] == tuple.get_value(*schema, i));
                }
            }

            {
                // Reinsert data into table, ought to be unsuccessful
                std::unique_ptr<Lex> lex = build_lex(file_path.c_str());
                EXPECT_FALSE(sql_execute(*lex).is_success());
            }

            db_manager.reset(nullptr); // Shutdown
        }

        // Reboot db and find this data successfully
        {
            db_manager = std::make_unique<DBManager>(meta, false);
            ASSERT_TRUE(db_manager->get_status());

            // Ensure we create table manually successfully 
            Catalog* catalog = db_manager->get_catalog();
            CatalogTable* catalog_tb = catalog->get_catalog_table();
            ASSERT_NE(catalog_tb->get_table_id(tb_name), -1);

            // Key value is set manually corresponding to the file parser_test3, default key value index is 0
            Value key_value(1);
            TableMetaData* table_meta = catalog_tb->get_table_meta_data(tb_name);
            const Schema* schema = table_meta->get_table_schema();
            Table* table = table_meta->get_table();
            ASSERT_NE(table, nullptr);
            Tuple tuple;
            ASSERT_TRUE(table->get_tuple(key_value, &tuple, *schema)); // Find this data

            // These values are set manually corresponding to the file parser_test3
            std::vector<Value> values{Value(1), Value("1234"), Value(23.33), Value(true)};

            // Check
            size_t size = values.size();
            for (size_t i = 0; i < size; i++) {
                EXPECT_TRUE(values[i] == tuple.get_value(*schema, i));
            }

            db_manager.reset(nullptr); // Shutdown
        }
    }
}

} // namespace dawn
