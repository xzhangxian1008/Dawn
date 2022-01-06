#pragma once

#include "ast/node.h"

namespace dawn {

enum DMLType : int8_t {
    kSelect,
    kInsert,
    kDelete,
    kValueList,
    kValue,
    kConstant
};

class DMLNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(DMLNode);
    DMLNode(DMLType type) : type_(type), Node(NodeType::kDML) {}

    /** We have to dynamic_cast this node in the outside */
    Node* get_node() const { return at(0); }

    DMLType get_dml_type() const { return type_; }
private:
    DMLType type_;
};

class ConstantNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(ConstantNode);
    ConstantNode() : type_(TypeId::kInteger), DMLNode(DMLType::kConstant) {}

    ~ConstantNode() override {
        if (type_ == TypeId::kChar)
            delete data_.str_;
    }

    TypeId get_type() const { return type_; }
    integer_t get_integer() const { return data_.integer_; }
    boolean_t get_boolean() const { return data_.boolean_; }
    decimal_t get_decimal() const { return data_.decimal_; }
    char* get_str() const { return data_.str_; }

    void set_integer(integer_t integer) { data_.integer_ = integer; }
    void set_boolean(boolean_t boolean) { data_.boolean_ = boolean; }
    void set_decimal(decimal_t decimal) { data_.decimal_ = decimal; }
    void set_str(char* str) { data_.str_ = str; }
private:
    TypeId type_;
    union {
        integer_t integer_;
        boolean_t boolean_;
        decimal_t decimal_;
        char* str_;
    } data_;
};

/**
 * It has only one child that contain the value:
 *   - idx 0: ConstantNode: contain the value
 */
class ValueNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(ValueNode);
    ValueNode(ConstantNode* node) : DMLNode(DMLType::kValue) {
        add_child(node);
    }

    // TODO some operations
};

/**
 * Contain values
 */
class ValueListNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(ValueListNode);
    ValueListNode() : DMLNode(DMLType::kValueList) {}

    // TODO some operations
};

/**
 * It has two children:
 *   - idx 0: IdentityNode: save the table name
 *   - idx 1: ValueListNode: save inserted values
 */
class InsertNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(InsertNode);
    InsertNode(IdentityNode* tb_name, ValueListNode* value_list)
        : DMLNode(DMLType::kInsert)
    {
        add_child(tb_name);
        add_child(value_list);
    }

    // TODO
};

} // namespace dawn
