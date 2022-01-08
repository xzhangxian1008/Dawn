#pragma once

#include "ast/node.h"
#include "data/values.h"

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

    TypeId get_dml_type() const { return type_; }
    integer_t get_integer() const { return data_.integer_; }
    boolean_t get_boolean() const { return data_.boolean_; }
    decimal_t get_decimal() const { return data_.decimal_; }
    char* get_str() const { return data_.str_; }
    size_t_ get_str_len() const { return str_len_; }

    void set_integer(integer_t integer) { data_.integer_ = integer; }
    void set_boolean(boolean_t boolean) { data_.boolean_ = boolean; }
    void set_decimal(decimal_t decimal) { data_.decimal_ = decimal; }
    void set_str(char* str, size_t_ str_len) {
        data_.str_ = str;
        str_len_ = str_len;
    }

    void set_value(Value* value) {
        switch (type_) {
        case TypeId::kInteger:
            value->construct(data_.integer_);
            break;
        case TypeId::kDecimal:
            value->construct(data_.decimal_);
            break;
        case TypeId::kBoolean:
            value->construct(data_.boolean_);
            break;
        case TypeId::kChar:
            value->construct(data_.str_);
        default:
            break;
        }
    }
private:
    TypeId type_;
    union {
        integer_t integer_;
        boolean_t boolean_;
        decimal_t decimal_;
        char* str_;
    } data_;
    size_t_ str_len_;
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

    /**
     * Pass value to reduce overhead,
     * or we will new char* twice.
     */
    void set_value(Value* value) const {
        ConstantNode* constant_node = dynamic_cast<ConstantNode*>(at(0));
        assert(constant_node);

        constant_node->set_value(value);
    }
};

/**
 * Contain values with ValueNode type
 */
class ValueListNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(ValueListNode);
    ValueListNode() : DMLNode(DMLType::kValueList) {}

    /** We do not construct values here in consideration of the copy cost */
    std::vector<Node*> get_values() const {
        return get_children();
    }
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

    string_t get_tb_name() const {
        IdentityNode* id_node = dynamic_cast<IdentityNode*>(at(0));
        assert(id_node);

        return id_node->get_id();
    }

    /** Prepare for tuples because constructing tuples needs Value */
    std::vector<Value> get_values() const {
        std::vector<Value> values;
        ValueListNode* value_list_node = dynamic_cast<ValueListNode*>(at(1));
        assert(value_list_node);

        std::vector<Node*> children = value_list_node->get_values();
        values.reserve(children.size());

        size_t size = children.size();
        for (size_t i = 0; i < size; i++) {
            ValueNode* value_node = dynamic_cast<ValueNode*>(children[i]);
            assert(value_node);

            value_node->set_value(&(values[i]));
        }
        
        return values;
    }
};

} // namespace dawn
