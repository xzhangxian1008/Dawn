#pragma once

#include <stdint.h>
#include <assert.h>
#include <utility>

#include "data/types.h"
#include "ast/node.h"
#include "util/config.h"

namespace dawn {

class CreateNode;
class CreateDefListNode;
class DDLNode;
class ColumnDefNode;

enum DDLType : int8_t {
    kCreateTable = 0,
    kDropTable,
    kCreateDefList,
    kCreateDef,
    kColumnDef
};

class DDLNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(DDLNode);
    DDLNode(DDLType type) : type_(type), Node(NodeType::kDDL) {}

    /** We have to dynamic_cast this node in the outside */
    Node* get_node() const { return at(0); }

    DDLType get_ddl_type() const { return type_; }
private:
    DDLType type_;
};

/**
 * It has only one child:
 *   - idx 0: DataTypeNode: save the data type
 */
class ColumnDefNode : public DDLNode {
public:
    DISALLOW_COPY_AND_MOVE(ColumnDefNode);
    ColumnDefNode(DataTypeNode* data_type)
        : DDLNode(DDLType::kColumnDef)
    {
        add_child(data_type);
    }
    
    TypeId get_data_type() const {
        DataTypeNode* data_node = get_data_type_node();
        return data_node->get_data_type();
    }

    // FIXME I think it's bad to call get_data_type_node() recursively.
    size_t_ get_char_len() const {
        DataTypeNode* data_node = get_data_type_node();
        if (data_node->get_data_type() != TypeId::kChar) {
            return -1;
        }

        return data_node->get_char_len();
    }

private:
    DataTypeNode* get_data_type_node() const {
        Node* node = at(0);
        DataTypeNode* data_node = dynamic_cast<DataTypeNode*>(node);
        assert(data_node);

        return data_node;
    }
};

/**
 * CreateDefNode has two types:
 *   - kColumn: refer to a column that specify col_name and column_def
 *   - kPrimaryKey: designate the primary key that must exists in fields
 * 
 * When it's kColumn type, it has two children:
 *   - idx 0: IdentityNode: save the column's name
 *   - idx 1: ColumnDefNode: save the colums's type
 * 
 * When it's kPrimaryKey type, it has only one child:
 *   - idx 0: IdentityNode: save the primary key's column name
 */
class CreateDefNode : public DDLNode {
public:
    enum class CreateDefNodeType : int8_t {
        kColumn,
        kPrimaryKey
    };

    DISALLOW_COPY_AND_MOVE(CreateDefNode);

    CreateDefNode(IdentityNode* col_name, ColumnDefNode* col_type)
        : type_(CreateDefNodeType::kColumn), DDLNode(DDLType::kCreateDef)
    {
        add_child(col_name);
        add_child(col_type);
    }

    CreateDefNode(IdentityNode* primary_key)
        : type_(CreateDefNodeType::kPrimaryKey), DDLNode(DDLType::kCreateDef)
    {
        add_child(primary_key);
    }

    string_t get_col_name() const {
        Node* node = at(0);
        IdentityNode* id_node = dynamic_cast<IdentityNode*>(node);
        assert(id_node);

        return id_node->get_id();
    }

    TypeId get_col_type() const {
        Node* node = at(1);
        ColumnDefNode* col_def_node = dynamic_cast<ColumnDefNode*>(node);
        assert(col_def_node);

        return col_def_node->get_data_type();
    }

    size_t_ get_char_len() const {
        Node* node = at(1);
        ColumnDefNode* col_def_node = dynamic_cast<ColumnDefNode*>(node);
        assert(col_def_node);

        return col_def_node->get_char_len();
    }

    CreateDefNodeType get_node_type() const { return type_; }
private:
    CreateDefNodeType type_;
};

/**
 * It has many CreateDefNode children.
 * Initialized-on-need: CreateDefListNode will be initialized only when it's needed
 */
class CreateDefListNode : public DDLNode {
public:
    DISALLOW_COPY_AND_MOVE(CreateDefListNode);
    CreateDefListNode()
        : DDLNode(DDLType::kCreateDefList), 
        primary_key_col_name_(""),
        initialized_(false) {}

    std::vector<string_t> get_col_name() const {
        initialize();
        return col_names_;
    }

    std::vector<TypeId> get_col_type() const {
        initialize();
        return col_types_;
    }

    string_t get_primary_key() const {
        initialize();
        return primary_key_col_name_;
    }

    std::vector<size_t_> get_char_len() const {
        initialize();
        return char_lens_;
    }

private:
    /**
     * @brief initialize names, types and primary key
     */
    void initialize() const {
        if (initialized_) {
            return;
        }
        initialized_ = true;
        const std::vector<Node*> children = get_children();
        for (Node* child : children) {
            CreateDefNode* node = dynamic_cast<CreateDefNode*>(child);
            assert(node);

            if (node->get_node_type() == CreateDefNode::CreateDefNodeType::kPrimaryKey) {
                // TODO handle primary key in the later
                // So far, we set the first column as the primary key
                continue;
            }

            // initialize names and types
            col_names_.push_back(node->get_col_name());
            col_types_.push_back(node->get_col_type());

            // get char length
            if (node->get_col_type() == TypeId::kChar) {
                char_lens_.push_back(node->get_char_len());
            }
        }
    }

    mutable string_t primary_key_col_name_;
    mutable bool initialized_;
    mutable std::vector<string_t> col_names_;
    mutable std::vector<TypeId> col_types_;
    mutable std::vector<size_t_> char_lens_;
};

/**
 * So far, it has two children:
 *   - idx 0: IdentityNode: save the table name
 *   - idx 1: CreateDefListNode: save the table's columns
 * 
 * It will have more types such as kCreateDatabase etc.
 */
class CreateNode : public DDLNode {
public:
    DISALLOW_COPY_AND_MOVE(CreateNode);
    CreateNode(IdentityNode* id_node, CreateDefListNode* create_def_list_node)
        : DDLNode(DDLType::kCreateTable)
    {
        add_child(id_node);
        add_child(create_def_list_node);
    }

    string_t get_tb_name() const {
        IdentityNode* id_node = dynamic_cast<IdentityNode*>(at(0));
        assert(id_node);

        return id_node->get_id();
    }

    std::vector<string_t> get_col_name() const {
        CreateDefListNode* create_def_list = get_create_def_list();
        return create_def_list->get_col_name();
    }

    std::vector<TypeId> get_col_type() const {
        CreateDefListNode* create_def_list = get_create_def_list();
        return create_def_list->get_col_type();
    }

    /**
     * @brief return length of chars in order
     * @return std::vector<size_t> save length of chars in order
     */
    std::vector<size_t_> get_char_len() const {
        CreateDefListNode* create_def_list = get_create_def_list();
        return create_def_list->get_char_len();
    }

    string_t get_primary_key() const {
        CreateDefListNode* create_def_list = get_create_def_list();
        return create_def_list->get_primary_key();
    }

private:
    CreateDefListNode* get_create_def_list() const {
        CreateDefListNode* create_def_list = dynamic_cast<CreateDefListNode*>(at(1));
        assert(create_def_list);
        return create_def_list;
    }
};

/**
 * So far, it has only one child:
 *   - idx 0: IdentityNode: save the table name
 * 
 * It will have more types such as kDropDatabase etc.
 */
class DropNode : public DDLNode {
public:
    DISALLOW_COPY_AND_MOVE(DropNode);
    DropNode(IdentityNode* id_node)
        : DDLNode(DDLType::kDropTable)
    {
        add_child(id_node); // add dropped table name
    }

    string_t get_tb_name() const {
        IdentityNode* id_node = dynamic_cast<IdentityNode*>(at(0));
        assert(id_node);

        return id_node->get_id();
    }
};

} // namespace dawn
