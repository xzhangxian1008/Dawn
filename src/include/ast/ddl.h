#pragma once

#include <stdint.h>
#include <assert.h>

#include "data/types.h"
#include "ast/node.h"
#include "util/config.h"

namespace dawn {

class CreateNode;
class CreateDefListNode;
class DDLNode;

enum DDLType : int8_t {
    kCreateTable,
    kDrop,
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
 * It has many CreateDefNode children
 */
class CreateDefListNode : public DDLNode {
public:
    DISALLOW_COPY_AND_MOVE(CreateDefListNode);
    CreateDefListNode() : DDLNode(DDLType::kCreateDefList) {}

    // TODO get children
};

/**
 * So far, it has two children:
 *   - idx 0: IdentityNode: save the table name
 *   - idx 1: CreateDefListNode: save the table's fields
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

    // TODO get table's fields
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
        Node* node = at(0);
        DataTypeNode* data_node = dynamic_cast<DataTypeNode*>(node);
        assert(data_node);

        return data_node->get_data_type();
    }
};

/**
 * CreateDefNode has two types:
 *   - kField: refer to a column that specify col_name and column_def
 *   - kPrimaryKey: designate the primary key that must exists in fields
 * 
 * When it's kField type, it has two children:
 *   - idx 0: IdentityNode: save the column's name
 *   - idx 1: ColumnDefNode: save the colums's type
 * 
 * When it's kPrimaryKey type, it has only one child:
 *   - idx 0: IdentityNode: save the primary key's column name
 */
class CreateDefNode : public DDLNode {
public:
    enum class CreateDefNodeType : int8_t {
        kField,
        kPrimaryKey
    };

    DISALLOW_COPY_AND_MOVE(CreateDefNode);

    CreateDefNode(IdentityNode* col_name, ColumnDefNode* col_type)
        : type_(CreateDefNodeType::kField), DDLNode(DDLType::kCreateDef)
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
private:
    CreateDefNodeType type_;
};

} // namespace dawn
