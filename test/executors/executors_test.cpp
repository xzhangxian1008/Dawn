#include "gtest/gtest.h"
#include "manager/db_manager.h"
#include "executors/seq_scan_executor.h"
#include "executors/proj_executor.h"
#include "plans/expressions/col_value_expr.h"

namespace dawn {

extern DBManager *db_manager;

/**
 * table name: table
 * column names:
 * ---------------------------------------------------
 * | tb_col1 | tb_col2 | tb_col3 | tb_col4 | tb_col5 |
 * ---------------------------------------------------
 * column types:
 * ---------------------------------------------------
 * | integer | char (5) | bool | char (10) | decimal | 
 * ---------------------------------------------------
 */
string_t table_name("table");
std::vector<TypeId> tb_col_types{TypeId::INTEGER, TypeId::CHAR, TypeId::BOOLEAN, TypeId::CHAR, TypeId::DECIMAL};
std::vector<string_t> tb_col_names{"tb_col1", "tb_col2", "tb_col3", "tb_col4", "tb_col5"};
size_t_ tb_char0_sz = 5;
size_t_ tb_char1_sz = 10;
std::vector<size_t_> tb_char_size{tb_char0_sz, tb_char1_sz};
size_t_ tb_tuple_size = Type::get_integer_size() + tb_char0_sz + Type::get_bool_size() + tb_char1_sz + Type::get_decimal_size();

/**
 * This is the projection executor's output table
 * table name: table2
 * column names:
 * ---------------------
 * | tb_col2 | tb_col5 |
 * ---------------------
 * column types:
 * ----------------------
 * | char (5) | decimal | 
 * ----------------------
 */
string_t table_name2("table2"); // output table by the projection executor
std::vector<TypeId> tb2_col_types{TypeId::CHAR, TypeId::DECIMAL};
std::vector<string_t> tb2_col_names{"tb_col2", "tb_col5"};
size_t_ tb2_char0_sz = 5;
std::vector<size_t_> tb2_char_size{tb2_char0_sz};
size_t_ tb2_tuple_size = tb2_char0_sz + Type::get_decimal_size();

const char *meta = "test";
const char *mtdf = "test.mtd";
const char *dbf = "test.db";
const char *logf = "test.log";

integer_t v0;
char v1[6];
boolean_t v2;
char v3[11];
decimal_t v4;
std::vector<Value> values;

class ExecutorsBasicTest : public testing::Test {
public:
    size_t_ default_pool_sz = 50;

    void SetUp() {
        values.clear();
        remove(mtdf);
        remove(dbf);
        remove(logf);
    }

    void TearDown() {
        remove(mtdf);
        remove(dbf);
        remove(logf);
    }
};

// ensure we can get all the data through the SeqScanExecutor
TEST_F(ExecutorsBasicTest, SeqScanExecutorBasicTest) {
    PRINT("start the basic SeqScanExecutor test...");
    Schema *tb_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);
    offset_t key_idx = tb_schema->get_key_idx();
    size_t_ insert_num = 12345;
    std::set<Value> insert_key_values;

    db_manager = new DBManager(meta, true);
    ASSERT_TRUE(db_manager->get_status());

    Catalog *catalog = db_manager->get_catalog();
    CatalogTable *catalog_table = catalog->get_catalog_table();
    ASSERT_TRUE(catalog_table->create_table(table_name, *tb_schema));

    TableMetaData *table_md = catalog_table->get_table_meta_data(table_name);
    ASSERT_NE(nullptr, table_md);

    Table *table = table_md->get_table();
    ASSERT_NE(nullptr, table);

    v0 = 2333;
    fill_char_array("apple", v1);
    v2 = true;
    fill_char_array("monkey_key", v3);
    v4 = 3.1415926;

    values.clear();
    values.push_back(Value(v0));
    values.push_back(Value(v1, tb_char0_sz));
    values.push_back(Value(v2));
    values.push_back(Value(v3, tb_char1_sz));
    values.push_back(Value(v4));

    // insert a lot of tuples
    PRINT("insert a lot of tuples...");
    bool ok = true;
    for (size_t_ i = 0; i < insert_num; i++) {
        values[key_idx] = Value(static_cast<integer_t>(i));
        Tuple tuple(&values, *tb_schema);
        if (!table->insert_tuple(&tuple, *tb_schema)) {
            ok = false;
            break;
        }
        insert_key_values.insert(values[key_idx]);
    }
    ASSERT_TRUE(ok);

    ExecutorContext *exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
    SeqScanExecutor seq_scan_exec(exec_ctx, table);

    seq_scan_exec.open();

    PRINT("get tuples with SeqScanExecutor...");
    size_t_ cnt = 0; // count how many tuples the iterator gets
    Tuple tuple;
    while (seq_scan_exec.get_next(&tuple)) {
        Value key_value = tuple.get_value(*tb_schema, key_idx);
        auto iter = insert_key_values.find(key_value);
        if (iter == insert_key_values.end()) {
            ok = false;
            break;
        }
        ++cnt;
    }
    ASSERT_TRUE(ok);
    ASSERT_EQ(cnt, insert_key_values.size());

    delete tb_schema;
    delete exec_ctx;
    delete db_manager;
    PRINT("***basic SeqScanExecutor test pass***");
}

// ProjectionExecutor may need more tests
TEST_F(ExecutorsBasicTest, ProjectionExecutorBasicTest) {
    PRINT("start the basic ProjectionExecutor test...");

    // projection executor's input schema and it's seqscan executor's schema
    Schema *input_tb_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);

    // projection executor's output schema
    Schema *output_tb_schema = create_table_schema(tb2_col_types, tb2_col_names, tb2_char_size);

    db_manager = new DBManager(meta, true);
    ASSERT_TRUE(db_manager->get_status());

    Catalog *catalog = db_manager->get_catalog();
    CatalogTable *catalog_table = catalog->get_catalog_table();
    ASSERT_TRUE(catalog_table->create_table(table_name, *input_tb_schema));

    TableMetaData *table_md = catalog_table->get_table_meta_data(table_name);
    ASSERT_NE(nullptr, table_md);

    Table *table = table_md->get_table();
    ASSERT_NE(nullptr, table);

    v0 = 2333;
    fill_char_array("apple", v1);
    v2 = true;
    fill_char_array("monkey_key", v3);
    v4 = 3.1415926;

    values.clear();
    values.push_back(Value(v0));
    values.push_back(Value(v1, tb_char0_sz));
    values.push_back(Value(v2));
    values.push_back(Value(v3, tb_char1_sz));
    values.push_back(Value(v4));

    Tuple tuple(&values, *input_tb_schema);
    ASSERT_TRUE(table->insert_tuple(&tuple, *input_tb_schema));

    // construct the SeqScanExecutor which is used for providing tuple for the ProjectionExecutor
    ExecutorContext *seq_scan_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
    SeqScanExecutor seq_scan_exec(seq_scan_exec_ctx, table);

    // construct the ProjectionExecutor
    std::vector<ExpressionAbstract*> exprs;
    exprs.push_back(new ColumnValueExpression(1));
    exprs.push_back(new ColumnValueExpression(4));

    ExecutorContext *proj_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
    ProjectionExecutor proj_exec(proj_exec_ctx, &seq_scan_exec, exprs, input_tb_schema, output_tb_schema);

    // check the ProjectionExecutor could work correctly
    proj_exec.open();
    ASSERT_TRUE(proj_exec.get_next(&tuple));
    ASSERT_FALSE(proj_exec.get_next(&tuple));

    EXPECT_EQ(tuple.get_size(), tb2_tuple_size);
    EXPECT_EQ(tuple.get_value(*output_tb_schema, 0), values[1]);
    EXPECT_EQ(tuple.get_value(*output_tb_schema, 1), values[4]);

    proj_exec.close();

    delete input_tb_schema;
    delete output_tb_schema;
    delete seq_scan_exec_ctx;
    delete proj_exec_ctx;
    for (auto p : exprs)
        delete p;
    delete db_manager;
    PRINT("***basic ProjectionExecutor test pass***");
}
    
} // namespace dawn
