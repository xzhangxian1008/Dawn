#include "sql/sql_parse.h"

void* dawn_parseAlloc(void*(*)(size_t));
void dawn_parseFree(void*p, void(*freeProc)(void*));
void dawn_parse(void*, int, dawn::Token, dawn::StmtListNode*);

namespace dawn {

extern std::unique_ptr<DBManager> db_manager;

bool create_table(CreateNode* node) {
    Catalog* catalog = db_manager->get_catalog();
    CatalogTable* catalog_tb = catalog->get_catalog_table();
    string_t tb_name = node->get_tb_name();

    // check duplication of table name
    page_id_t page_id = catalog_tb->get_table_id(tb_name);
    if (page_id != -1) {
        // duplicated table name
        return false;
    }

    // initialize table schema
    std::vector<TypeId> col_types = node->get_col_type();
    std::vector<string_t> col_names = node->get_col_name();
    std::vector<size_t_> char_len = node->get_char_len();
    std::unique_ptr<Schema> schema(create_table_schema(col_types, col_names, char_len));

    if (!catalog_tb->create_table(tb_name, *schema)) {
        FATAL("Create table fails");
    }
    
    return true;
}

bool drop_table(DropNode* node) {
    Catalog* catalog = db_manager->get_catalog();
    CatalogTable* catalog_tb = catalog->get_catalog_table();
    string_t tb_name = node->get_tb_name(); // get dropped table's name

    return catalog_tb->delete_table(tb_name);
}

bool insert_data(InsertNode* node) {
    Catalog* catalog = db_manager->get_catalog();
    CatalogTable* catalog_tb = catalog->get_catalog_table();
    string_t tb_name = node->get_tb_name(); // get inserted table's name
    TableMetaData* table_meta = catalog_tb->get_table_meta_data(tb_name);

    if (table_meta == nullptr)
        return false;

    // get inserted table
    const Schema* schema = table_meta->get_table_schema();
    Table* table = table_meta->get_table();

    // construct Tuple from InsertNode
    std::vector<Value> values =  node->get_values();
    Tuple tuple(&values, *schema);

    return table->insert_tuple(&tuple, *schema);
}

bool sql_execute(Lex& lex) {
    void* parser = dawn_parseAlloc(malloc);
    StmtListNode* ast_root = new StmtListNode;
    Token tk;

    while (lex.next_token(&tk)) {
        dawn_parse(parser, tk.type_, tk, ast_root);
    }

    dawn_parse(parser, 0, tk, ast_root);

    bool success = false;

    if (ast_root->is_error()) {
        dawn_parseFree(parser, free);
        delete ast_root;
        return success;
    }
    
    std::vector<StmtNode*> stmt_nodes = ast_root->get_stmt_nodes();
    for (StmtNode* stmt : stmt_nodes) {
        if (stmt->get_type() == NodeType::kEmptyStmt) continue;

        if (DDLNode* ddl_node = dynamic_cast<DDLNode*>(stmt)) {
            DDLType ddl_type = ddl_node->get_ddl_type();
            switch (ddl_type) {
            case DDLType::kCreateTable: {
                CreateNode* node = dynamic_cast<CreateNode*>(ddl_node);
                assert(node);
                success = create_table(node);
                break;
            }
            case DDLType::kDropTable:{
                DropNode* node = dynamic_cast<DropNode*>(ddl_node);
                assert(node);
                success = drop_table(node);
                break;
            }
            default:
                // invalid ddl type
                assert(0);
            }
        } else if (DMLNode* dml_node = dynamic_cast<DMLNode*>(stmt)) {
            DMLType dml_type = dml_node->get_dml_type();
            switch (dml_type) {
            case DMLType::kInsert:{
                InsertNode* node = dynamic_cast<InsertNode*>(dml_node);
                assert(node);
                success = insert_data(node);
                break;
            }
            default:
                // invalid dml type
                assert(0);
            }
        } else {
            // invalid node
            assert(0);
        }
    }

    dawn_parseFree(parser, free);
    delete ast_root;
    return success;
}
    
} // namespace dawn
