#include <string>
#include <vector>

#include "ast/node.h"
#include "manager/db_manager.h"
#include "meta/catalog.h"
#include "table/schema.h"
#include "table/column.h"

extern FILE* yyin;
int yyparse();

// This is bad because multi sql parsers will use this variable
// TODO Modify this when we need to run in the multi-thread environment
extern dawn::StmtListNode* ast_root

namespace dawn {

extern std::unique_ptr<DBManager> db_manager;

bool create_table(CreateNode* node) {
    Catalog* catalog = db_manager->get_catalog();
    CatalogTable* catalog_tb = catalog->get_catalog_table();

    // check duplication of table name
    page_id_t page_id = catalog_tb->get_table_id(node->get_tb_name());
    if (page_id != -1) {
        // duplicated table name
        return false;
    }

    // TODO create table

}

/**
 * @brief So far, I have no idea that how to set string as the source for yyin.
 *        And this is the temporary method to save sql commands in files.
 * @param file_name file that contains sql commands
 */
bool sql_execute(std::string file_name) {
    std::string file_path("file_name");
    yyin = fopen(file_path.data(),"r");
    if (yyin == nullptr) {
        return false;
    }

    /**
     * If there are several sqls in the file, all should be
     * parsed successfully at the same time, or yyparse()
     * will not return 0.
     * 
     * This is bad and we should execute as many as possible.
     * The best way to handle this is to parse sql with string
     * , not file.
     */
    if (yyparse() != 0) {
        return false;
    }

    bool success = false;

    std::vector<StmtNode*> stmt_nodes = ast_root->get_stmt_nodes();
    for (StmtNode* stmt : stmt_nodes) {
        if (DDLNode* ddl_node = dynamic_cast<DDLNode*>(ddl_node)) {
            DDLType ddl_type = ddl_node->get_ddl_type();
            switch (ddl_type) {
            case DDLType::kCreateTable:
                CreateNode* node = dynamic_cast<CreateNode*>(ddl_node);
                assert(node);
                success = create_table(node);
                break;
            case DDLType::kDrop:
                success = false;
                break;
            default:
                // invalid ddl type
                assert(0);
            }
        } else if (DMLNode* dml_node = dynamic_cast<DMLNode*>(dml_node)) {
            // TODO handle the dml_node
            success = false;
        } else {
            // invalid node
            assert(0);
        }
    }

    delete ast_root;
    return success;
}
    
} // namespace dawn
