#include "gtest/gtest.h"
#include "manager/db_manager.h"
#include "plans/expressions/col_value_expr.h"
#include "plans/expressions/constant_expr.h"
#include "plans/expressions/comparison_expr.h"
#include "plans/expressions/aggregate_expr.h"
#include "executors/seq_scan_executor.h"
#include "executors/proj_executor.h"
#include "executors/selection_executor.h"
#include "executors/union_executor.h"

namespace dawn {

extern std::unique_ptr<DBManager> db_manager;

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

/**
 * This is the projection executor's output table for aggregation function test
 * table name: table3
 * column names:
 * -----------------------------------------------------------------------------------
 * | tb_col2 | tb_col5 | min(tb_col5) | max(tb_col5) | sum(tb_col5) | count(tb_col5) |
 * -----------------------------------------------------------------------------------
 * column types:
 * --------------------------------------------------------------
 * | char (5) | integer | integer | integer | integer | integer |
 * --------------------------------------------------------------
 */
string_t table_name3("table3"); // output table by the projection executor
std::vector<TypeId> tb3_col_types{TypeId::CHAR, TypeId::INTEGER, TypeId::INTEGER, TypeId::INTEGER, TypeId::INTEGER, TypeId::INTEGER};
std::vector<string_t> tb3_col_names{"tb_col2", "tb_col5", "min(tb_col5)", "max(tb_col5)", "sum(tb_col5)", "count(tb_col5)"};
size_t_ tb3_char0_sz = 5;
std::vector<size_t_> tb3_char_size{tb3_char0_sz};
size_t_ tb3_tuple_size = tb3_char0_sz + Type::get_decimal_size() * 4 + Type::get_integer_size();
offset_t tb3_min_idx = 2;
offset_t tb3_max_idx = 3;
offset_t tb3_sum_idx = 4;
offset_t tb3_cnt_idx = 5;

/**
 * table for the UnionExecutor test with few tuples inserted
 * table name: left_few_tp_table
 * column names:
 * ---------------------------------------------------
 * | left_few_tp_table_col1 | left_few_tp_table_col2 |
 * ---------------------------------------------------
 * column types:
 * ---------------------
 * | integer | decimal |
 * ---------------------
 */
string_t left_few_tp_table("left_few_tp_table");
std::vector<TypeId> left_few_tp_table_col_types{TypeId::INTEGER, TypeId::DECIMAL};
std::vector<string_t> left_few_tp_table_col_names{"left_few_tp_table_col1", "left_few_tp_table_col2"};
size_t_ left_few_tp_table_tuple_size = Type::get_integer_size() + Type::get_decimal_size();

/**
 * table for the UnionExecutor test with many tuples inserted
 * table name: left_many_tp_table
 * column names:
 * ---------------------------------------------------
 * | left_few_tp_table_col1 | left_few_tp_table_col2 |
 * ---------------------------------------------------
 * column types:
 * ---------------------
 * | integer | decimal |
 * ---------------------
 */
string_t left_many_tp_table("left_many_tp_table");
std::vector<TypeId> left_many_tp_table_col_types{TypeId::INTEGER, TypeId::DECIMAL};
std::vector<string_t> left_many_tp_table_col_names{"left_many_tp_table_col1", "left_many_tp_table_col2"};
size_t_ left_many_tp_table_tuple_size = Type::get_integer_size() + Type::get_decimal_size();

const char *meta = "test";
const char *mtdf = "test.mtd";
const char *dbf = "test.db";
const char *logf = "test.log";

/** For UnionExecutorTest */
const char *union_exec_meta = "union_exec_test";
const char *union_exec_mtdf = "union_exec_test.mtd";
const char *union_exec_dbf = "union_exec_test.db";
const char *union_exec_logf = "union_exec_test.log";


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

/**
 * set "left_few_tp_table" or "left_many_tp_table" as left table and "table" as right table
 * insert few tuples into left_few_tp_table
 * insert many tuples into left_many_tp_table
 */
class UnionExecutorTest : public testing::Test {
public:
    size_t_ left_table_tuple_num_1; // few tuples, for left_few_tp_table
    size_t_ left_table_tuple_num_2; // many tuples, for left_many_tp_table
    constexpr static size_t_ default_pool_sz = 200;
    constexpr static size_t_ right_table_tuple_num = 100;

    /** return the number of tuples inserted into the right table */
    constexpr size_t_ get_right_tuples_num() { return right_table_tuple_num; }

    inline size_t_ get_few_tuples_num() { return left_table_tuple_num_1; }
    inline size_t_ get_many_tuples_num() { return left_table_tuple_num_2; }

    /**
     * Before running the UnionExecutor test, we need to prepare some data for it.
     * If the preparation fails, the test should not start.
     */
    inline bool is_test_ready() { return ok; }

    /**
     * check the existence of meta_1 and meta_2 first,
     * create them when we can't find.
     */
    void SetUp() {
        ok = true;
        values.clear();

        // check the meta_1
        if (check_inexistence(union_exec_mtdf) ||
             check_inexistence(union_exec_dbf) ||
             check_inexistence(union_exec_logf)) {
            create_meta();
        }
    }

    /** do nothing */
    void TearDown() {}

private:
    static bool ok;

    // TODO delete all the files when create_meta() exit abnormally
    void create_meta() {
        PRINT("create files for the UnionExecutor test...");
        
        // remove, just in case
        remove(union_exec_mtdf);
        remove(union_exec_dbf);
        remove(union_exec_logf);

        DBManager::set_default_pool_size(default_pool_sz);
        db_manager.reset(new DBManager(union_exec_meta, true));
        ASSERT_TRUE(db_manager->get_status());

        {
            /** insert tuples into left_few_tp_table */
            size_t_ left_few_tp_table_num2page = TablePage::get_tp_num_capacity(left_few_tp_table_tuple_size); // number of table2's tuples a TablePage could contain
            left_table_tuple_num_1 = (default_pool_sz / 3) * left_few_tp_table_num2page;

            std::unique_ptr<Schema> tb_schema(create_table_schema(left_few_tp_table_col_types, left_few_tp_table_col_names));
            Catalog *catalog = db_manager->get_catalog();
            CatalogTable *catalog_table = catalog->get_catalog_table();
            catalog_table->create_table(left_few_tp_table, *tb_schema);
            TableMetaData *table_md = catalog_table->get_table_meta_data(left_few_tp_table);
            Table *table = table_md->get_table();

            v0 = 0;
            fill_char_array("apple", v1);
            v4 = 3.1415926;

            values.clear();
            values.push_back(Value(v0));
            values.push_back(Value(v1));
            values.push_back(Value(v4));

            for (size_t_ i = 0; i < left_table_tuple_num_1; i++) {
                values[0] = Value(static_cast<integer_t>(i));
                Tuple tuple(&values, *tb_schema);
                if (!table->insert_tuple(&tuple, *tb_schema)) {
                    ok = false;
                    return;
                }
            }
        }

        {
            /** insert tuples into left_many_tp_table */
            size_t_ tuple4_num2page = TablePage::get_tp_num_capacity(left_many_tp_table_tuple_size); // number of table4's tuples a TablePage could contain
            left_table_tuple_num_2 = 3 * default_pool_sz * tuple4_num2page;

            std::unique_ptr<Schema> tb_schema(create_table_schema(left_many_tp_table_col_types, left_many_tp_table_col_names));
            Catalog *catalog = db_manager->get_catalog();
            CatalogTable *catalog_table = catalog->get_catalog_table();
            catalog_table->create_table(left_many_tp_table, *tb_schema);
            TableMetaData *table_md = catalog_table->get_table_meta_data(left_many_tp_table);
            Table *table = table_md->get_table();

            integer_t v0 = 0;
            decimal_t v4 = 3.14;

            values.clear();
            values.push_back(Value(v0));
            values.push_back(Value(v4));

            for (size_t_ i = 0; i < left_table_tuple_num_2; i++) {
                values[0] = Value(static_cast<integer_t>(i));
                Tuple tuple(&values, *tb_schema);
                if (!table->insert_tuple(&tuple, *tb_schema)) {
                    ok = false;
                    return;
                }
            }
        }
    }
};

bool UnionExecutorTest::ok = true;

/** ensure we can get all the data through the SeqScanExecutor */
TEST_F(ExecutorsBasicTest, DISABLED_SeqScanExecutorBasicTest) {
    PRINT("start the basic SeqScanExecutor test...");
    Schema *tb_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);
    offset_t key_idx = tb_schema->get_key_idx();
    size_t_ insert_num = 12345;
    std::set<Value> insert_key_values;

    db_manager.reset(new DBManager(meta, true));
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
}

/**
 * Test List:
 *   1. test the basic function of the ProjectionExecutor
 *   2. test the aggregation function of the ProjectionExecutor
 * 
 * ProjectionExecutor may need more tests.
 * In the current test, we push only one tuple, one input and output schema through it.
 * Multiple tuples or other possible input and output schemas may need be tested.
 */
TEST_F(ExecutorsBasicTest, DISABLED_ProjectionExecutorBasicTest) {
    PRINT("start the basic ProjectionExecutor test...");

    // projection executor's input schema and it's seqscan executor's schema
    Schema *input_tb_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);

    db_manager.reset(new DBManager(meta, true));
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

    {
        // test 1

        // projection executor's output schema
        Schema *output_tb_schema = create_table_schema(tb2_col_types, tb2_col_names, tb2_char_size);

        Tuple tuple(&values, *input_tb_schema);
        ASSERT_TRUE(table->insert_tuple(&tuple, *input_tb_schema));

        // construct the SeqScanExecutor which is used for providing tuple for the ProjectionExecutor
        ExecutorContext *seq_scan_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SeqScanExecutor seq_scan_exec(seq_scan_exec_ctx, table);

        // // construct the ProjectionExecutor
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

        ASSERT_TRUE(table->mark_delete(Value(v0), *input_tb_schema));
        table->apply_delete(Value(v0), *input_tb_schema);

        delete output_tb_schema;
        delete seq_scan_exec_ctx;
        delete proj_exec_ctx;
        for (auto p : exprs)
            delete p;
        PRINT("basic function of the ProjectionExecutor is ok...");
    }

    {
        // test 2

        // projection executor's output schema
        Schema *output_tb_schema = create_table_schema(tb3_col_types, tb3_col_names, tb3_char_size);

        offset_t key_idx = input_tb_schema->get_key_idx();
        size_t_ insert_num = 10000;
        std::set<Value> insert_key_values;

        PRINT("insert a lot of tuples...");
        bool ok = true;
        for (size_t_ i = 0; i < insert_num; i++) {
            values[key_idx] = Value(static_cast<integer_t>(i));
            Tuple tuple(&values, *input_tb_schema);
            if (!table->insert_tuple(&tuple, *input_tb_schema)) {
                ok = false;
                break;
            }
            insert_key_values.insert(values[key_idx]);
        }
        ASSERT_TRUE(ok);

        // construct the SeqScanExecutor which is used for providing tuple for the ProjectionExecutor
        ExecutorContext *seq_scan_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SeqScanExecutor seq_scan_exec(seq_scan_exec_ctx, table);

        // construct the ProjectionExecutor
        std::vector<ExpressionAbstract*> exprs;
        exprs.push_back(new ColumnValueExpression(1));
        exprs.push_back(new ColumnValueExpression(4));

        std::vector<ExpressionAbstract*> agg_expr;
        agg_expr.push_back(new AggregateExpression(AggregationType::MinAggregate, key_idx));
        agg_expr.push_back(new AggregateExpression(AggregationType::MaxAggregate, key_idx));
        agg_expr.push_back(new AggregateExpression(AggregationType::SumAggregate, key_idx));
        agg_expr.push_back(new AggregateExpression(AggregationType::CountAggregate, key_idx));

        std::vector<AggregationType> agg_type;
        agg_type.push_back(AggregationType::MinAggregate);
        agg_type.push_back(AggregationType::MaxAggregate);
        agg_type.push_back(AggregationType::SumAggregate);
        agg_type.push_back(AggregationType::CountAggregate);
        
        ExecutorContext *proj_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        ProjectionExecutor proj_exec(proj_exec_ctx, &seq_scan_exec, exprs, agg_type, agg_expr, input_tb_schema, output_tb_schema);

        // values for validation
        Value min_num(static_cast<integer_t>(0));
        Value max_num(static_cast<integer_t>(insert_num - 1));
        Value sum_num(static_cast<integer_t>(((insert_num-1) * insert_num) / 2));
        Value cnt_num(static_cast<integer_t>(insert_num));

        Tuple tuple;
        int cnt = 0;
        key_idx = output_tb_schema->get_key_idx();
        proj_exec.open();
        while (proj_exec.get_next(&tuple)) {
            Value key_value = tuple.get_value(*output_tb_schema, key_idx);
            auto iter = insert_key_values.find(key_value);
            if (iter == insert_key_values.end()) {
                ok = false;
                break;
            }

            /** these commented codes are used for debugging */
            // if (tuple.get_value(*output_tb_schema, tb3_min_idx) != min_num) {
            //     PRINT(tuple.get_value(*output_tb_schema, tb3_min_idx).get_value_string());
            //     LOG("HERE");
            // }

            // if (tuple.get_value(*output_tb_schema, tb3_max_idx) != max_num) {
            //     PRINT(tuple.get_value(*output_tb_schema, tb3_max_idx).get_value_string());
            //     LOG("HERE");
            // }

            // if (tuple.get_value(*output_tb_schema, tb3_sum_idx) != sum_num) {
            //     PRINT(tuple.get_value(*output_tb_schema, tb3_sum_idx).get_value_string());
            //     PRINT(sum_num.get_value_string());
            //     LOG("HERE");
            // }

            // if (tuple.get_value(*output_tb_schema, tb3_cnt_idx) != cnt_num) {
            //     PRINT(tuple.get_value(*output_tb_schema, tb3_cnt_idx).get_value_string());
            //     LOG("HERE");
            // }

            // check the aggregation values
            if (tuple.get_value(*output_tb_schema, tb3_min_idx) != min_num ||
                tuple.get_value(*output_tb_schema, tb3_max_idx) != max_num ||
                tuple.get_value(*output_tb_schema, tb3_sum_idx) != sum_num ||
                tuple.get_value(*output_tb_schema, tb3_cnt_idx) != cnt_num) 
            {
                ok = false;
                break;
            }

            ++cnt;
        }
        EXPECT_TRUE(ok);
        EXPECT_EQ(cnt, insert_num);
        proj_exec.close();

        delete output_tb_schema;
        delete seq_scan_exec_ctx;
        delete proj_exec_ctx;
        for (auto p :exprs)
            delete p;
        for (auto p : agg_expr)
            delete p;
        PRINT("aggregation function of the ProjectionExecutor is ok...");
    }

    delete input_tb_schema;
}

/**
 * Test List:
 *   1. tuple     vs   constant
 *   2. value     vs   value (In the same tuple)
 *   3. tuple     vs   tuple (This needs subquery, ignore it so far)
 * 
 * In the current test, we only test the Integer type, other types may need tests,
 * but we suppose that they are ok when Integer type tests are passed.
 */
TEST_F(ExecutorsBasicTest, DISABLED_SelectionExecutorBasicTest) {
    PRINT("start the basic SelectionExecutorBasicTest test...");
    Schema *tb_schema = create_table_schema(tb_col_types, tb_col_names, tb_char_size);
    offset_t key_idx = tb_schema->get_key_idx();
    size_t_ insert_num = 12345;
    std::set<Value> insert_key_values;

    db_manager.reset(new DBManager(meta, true));
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

    {
        // where tb_col1 == 123

        offset_t cmp_idx = 0;
        integer_t target_num = 123;

        // prepare the SeqScanExecutor
        ExecutorContext *seq_scan_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SeqScanExecutor seq_scan_exec(seq_scan_exec_ctx, table);

        // prepare the Expression
        ColumnValueExpression col_expr(cmp_idx);
        ConstantExpression const_expr(target_num);
        std::vector<ExpressionAbstract*> cmp_children{&col_expr, &const_expr};
        ComparisonExpression cmp_expr(cmp_children, ComparisonType::Equal);

        // create the SelectionExecutor
        ExecutorContext *sel_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SelectionExecutor sel_exec(sel_exec_ctx, &cmp_expr, &seq_scan_exec, tb_schema);
        
        sel_exec.open();

        // start the operation...
        Tuple tuple;
        ASSERT_TRUE(sel_exec.get_next(&tuple));

        Value cmp_val(target_num);
        EXPECT_EQ(cmp_val, tuple.get_value(*tb_schema, cmp_idx));

        ASSERT_FALSE(sel_exec.get_next(&tuple));

        sel_exec.close();

        delete seq_scan_exec_ctx;
        delete sel_exec_ctx;
        PRINT("<where tb_col1 == 123> test ok...");
    }

    {
        // where tb_col1 != 2333

        offset_t cmp_idx = 0;
        integer_t target_num = 2333;

        // prepare the SeqScanExecutor
        ExecutorContext *seq_scan_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SeqScanExecutor seq_scan_exec(seq_scan_exec_ctx, table);

        // prepare the Expression
        ColumnValueExpression col_expr(cmp_idx);
        ConstantExpression const_expr(target_num);
        std::vector<ExpressionAbstract*> cmp_children{&col_expr, &const_expr};
        ComparisonExpression cmp_expr(cmp_children, ComparisonType::NotEqual);

        // create the SelectionExecutor
        ExecutorContext *sel_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SelectionExecutor sel_exec(sel_exec_ctx, &cmp_expr, &seq_scan_exec, tb_schema);
        
        sel_exec.open();

        // start the operation...
        int cnt = 0;
        int ok = true;
        Tuple tuple;
        Value tmp_val(target_num);
        while (sel_exec.get_next(&tuple)) {
            cnt++;
            if (tmp_val == tuple.get_value(*tb_schema, cmp_idx)) {
                ok = false;
                break;
            }
        }
        EXPECT_TRUE(ok);
        EXPECT_EQ(cnt, insert_num - 1);
        sel_exec.close();

        delete seq_scan_exec_ctx;
        delete sel_exec_ctx;
        PRINT("<where tb_col1 != 2333> test ok...");
    }

    {
        // where tb_col1 <= 2000

        offset_t cmp_idx = 0;
        integer_t target_num = 2000;

        // prepare the SeqScanExecutor
        ExecutorContext *seq_scan_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SeqScanExecutor seq_scan_exec(seq_scan_exec_ctx, table);

        // prepare the Expression
        ColumnValueExpression col_expr(cmp_idx);
        ConstantExpression const_expr(target_num);
        std::vector<ExpressionAbstract*> cmp_children{&col_expr, &const_expr};
        ComparisonExpression cmp_expr(cmp_children, ComparisonType::LessThanOrEqual);

        // create the SelectionExecutor
        ExecutorContext *sel_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SelectionExecutor sel_exec(sel_exec_ctx, &cmp_expr, &seq_scan_exec, tb_schema);
        
        sel_exec.open();

        // start the operation...
        int cnt = 0;
        int ok = true;
        Tuple tuple;
        Value cmp_val(target_num);
        Value get_val;

        while (sel_exec.get_next(&tuple)) {
            cnt++;
            get_val = tuple.get_value(*tb_schema, cmp_idx);
            if (get_val < cmp_val || get_val == cmp_val)
                continue;
            ok = false;
            break;
        }
        EXPECT_TRUE(ok);
        EXPECT_EQ(cnt, target_num + 1);
        sel_exec.close();

        delete seq_scan_exec_ctx;
        delete sel_exec_ctx;
        PRINT("<where tb_col1 <= 2000> test ok...");
    }

    {
        // where tb_col1 >= 10000

        offset_t cmp_idx = 0;
        integer_t target_num = 10000;

        // prepare the SeqScanExecutor
        ExecutorContext *seq_scan_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SeqScanExecutor seq_scan_exec(seq_scan_exec_ctx, table);

        // prepare the Expression
        ColumnValueExpression col_expr(cmp_idx);
        ConstantExpression const_expr(target_num);
        std::vector<ExpressionAbstract*> cmp_children{&col_expr, &const_expr};
        ComparisonExpression cmp_expr(cmp_children, ComparisonType::GreaterThanOrEqual);

        // create the SelectionExecutor
        ExecutorContext *sel_exec_ctx = new ExecutorContext(db_manager->get_buffer_pool_manager());
        SelectionExecutor sel_exec(sel_exec_ctx, &cmp_expr, &seq_scan_exec, tb_schema);

        sel_exec.open();

        // start the operation...
        int cnt = 0;
        int ok = true;
        Tuple tuple;
        Value cmp_val(target_num);
        Value get_val;

        while (sel_exec.get_next(&tuple)) {
            cnt++;
            get_val = tuple.get_value(*tb_schema, cmp_idx);
            if (get_val < cmp_val) {
                ok = false;
                break;
            }
        }
        EXPECT_TRUE(ok);
        EXPECT_EQ(cnt, insert_num - target_num);
        sel_exec.close();

        delete seq_scan_exec_ctx;
        delete sel_exec_ctx;
        PRINT("<where tb_col1 > 10000> test ok...");
    }

    delete tb_schema;
}


/**
 * FIXME bug here, ignore it so far
 * set "table2" or "table4" as left table and "table" as right table
 * insert few tuples into table2
 * insert many tuples into table4
 * 
 * Test List:
 *   FIXME we check the number of output tuples first, then check the content of the tuples
 *   1. all the inner tuples could be loaded in the memory and execute the same query for several times.
 *   2. too many inner tuples, spill them to the disk and execute the same query for several times.
 */
TEST_F(UnionExecutorTest, DISABLED_UnionExecutorBasicTest) {
    ASSERT_TRUE(UnionExecutorTest::is_test_ready());

    db_manager.reset(new DBManager(union_exec_meta, false));

    {
        // test 1
        Catalog *catalog = db_manager->get_catalog();
        CatalogTable *catalog_table = catalog->get_catalog_table();

        /** prepare the left table's sequence executor */
        TableMetaData *table_md = catalog_table->get_table_meta_data(table_name2);
        ASSERT_NE(nullptr, table_md);
        Table *left_table = table_md->get_table();
        ASSERT_NE(nullptr, left_table);

        ExecutorContext left_seq_exec_ctx(db_manager->get_buffer_pool_manager());
        SeqScanExecutor left_seq_scan_exec(&left_seq_exec_ctx, left_table);

        /** prepare the right table's sequence executor */
        table_md = catalog_table->get_table_meta_data(table_name);
        ASSERT_NE(nullptr, table_md);
        Table *right_table = table_md->get_table();
        ASSERT_NE(nullptr, right_table);

        ExecutorContext right_seq_exec_ctx(db_manager->get_buffer_pool_manager());
        SeqScanExecutor right_seq_scan_exec(&right_seq_exec_ctx, right_table);

        /** create the UnionExecutor */
        ExecutorContext union_exec_ctx(db_manager->get_buffer_pool_manager());
        std::vector<Schema*> child_schema;
        child_schema.push_back(create_table_schema(tb2_col_types, tb2_col_names, tb2_char_size)); // left child's schema
        child_schema.push_back(create_table_schema(tb_col_types, tb_col_names, tb_char_size)); // right child's schema

        UnionExecutor union_exec(&union_exec_ctx, 
            std::vector<ExecutorAbstract*>{&left_seq_scan_exec, &right_seq_scan_exec},
            child_schema);
        
        union_exec.open();

        int cnt = 0;
        Tuple output_tuple;

        // TODO so far, we only check the number of the output tuples, further checks will be done in the future
        while (union_exec.get_next(&output_tuple)) {
            cnt++;
        }

        union_exec.close();

        delete child_schema[0];
        delete child_schema[1];
        ASSERT_EQ(cnt, UnionExecutorTest::get_few_tuples_num() * UnionExecutorTest::get_right_tuples_num());
    }

    {
        // test 2
        
    }
}

} // namespace dawn
