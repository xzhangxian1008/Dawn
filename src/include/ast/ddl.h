#pragma once

#include <stdint.h>
#include "ast/node.h"

namespace dawn {

enum DDLType : int8_t {
    kCreateTable,
    kDelete,
    kCreateDefList,
    kCreateDef,
    kColumnDef
};

class DDLNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(DDLNode);
    DDLNode(DDLType type) : type_(type), Node(NodeType::kDDL) {}
private:
    DDLType type_;
};

/**
 * So far, it has three children:
 *   - idx 0: IdentityNode: save the table name
 *   - idx 1: CreateDefListNode: save the table's fields
 */
class CreateTableNode : public DDLNode {
public:
    DISALLOW_COPY_AND_MOVE(CreateTableNode);
    CreateTableNode() : DDLNode(DDLType::kCreateTable) {}
private:
};

class CreateDefListNode : public DDLNode {
public:
    DISALLOW_COPY_AND_MOVE(CreateDefListNode);
    CreateDefListNode() : DDLNode(DDLType::kCreateDefList) {}
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
        add(data_type);
    }
    
    DataType get_data_type() const {
        // TODO get data type
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

    CreateDefNode(IdentityNode col_name, ColumnDefNode col_type)
        : type_(CreateDefNodeType::kField), DDLNode(DDLType::kCreateDef)
    {
        add(col_name);
        add(col_type);
    }

    CreateDefNode(IdentityNode primary_key)
        : type_(CreateDefNodeType::kPrimaryKey), DDLNode(DDLType::kCreateDef)
    {
        add(primary_key);
    }

    std::string get_col_name() const {
        Node* node = at(0);
        // TODO get column name
    }

    DataType get_col_type() const {
        Node* node = at(1);
        // TODO get column type
    }
private:
    CreateDefNodeType type_;
};

} // namespace dawn
