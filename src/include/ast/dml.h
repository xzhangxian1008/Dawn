#pragma once

#include "ast/node.h"
#include "data/values.h"
#include "executors/executor_abstr.h"

namespace dawn {

using TableNameNode = IdentifierNode;

enum DMLType : int8_t {
    kSelect,
    kInsert,
    kDelete,
    kValueList,
    kValue,
    kConstant,
    kSelectExprList,
    kSelectExpr,
    kTableRefs,
    kTableRef,
    kTableFactor,
    kWhereCond,
    kExpr,
    kSimpleExpr,
    kBitExpr,
    kPredicate,
    kComparisonOpr,
    kBooleanPrimary
};

enum ExprType : int8_t {
    kPlus,
    kMinus,
    kMultiply,
    kDivide,
    kGreater,
    kLess,
    kGreaterEq,
    kLessEq,
    kEqual
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

class LiteralNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(LiteralNode);
    LiteralNode() : type_(TypeId::kInteger), DMLNode(DMLType::kConstant) {}

    ~LiteralNode() override {
        if (type_ == TypeId::kChar)
            delete[] data_.str_;
    }

    TypeId get_dml_type() const { return type_; }
    integer_t get_integer() const { return data_.integer_; }
    boolean_t get_boolean() const { return data_.boolean_; }
    decimal_t get_decimal() const { return data_.decimal_; }
    char* get_str() const { return data_.str_; }
    size_t_ get_str_len() const { return str_len_; }

    void set_integer(integer_t integer) {
        type_ = TypeId::kInteger;
        data_.integer_ = integer;
    }
    void set_boolean(boolean_t boolean) {
        type_ = TypeId::kBoolean;
        data_.boolean_ = boolean;
    }
    void set_decimal(decimal_t decimal) {
        type_ = TypeId::kDecimal;
        data_.decimal_ = decimal;
    }
    void set_str(char* str, size_t_ str_len) {
        type_ = TypeId::kChar;
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
            value->construct(data_.str_, str_len_);
            break;
        default:
            assert(0); // Invalid type
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
 *   - idx 0: LiteralNode: contain the value
 */
class ValueNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(ValueNode);
    ValueNode(LiteralNode* node) : DMLNode(DMLType::kValue) {
        add_child(node);
    }

    /**
     * Pass Value pointer to reduce overhead,
     * or we will new char* twice.
     */
    void set_value(Value* value) const {
        LiteralNode* literal_node = dynamic_cast<LiteralNode*>(at(0));
        assert(literal_node);

        literal_node->set_value(value);
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
 *   - idx 0: IdentifierNode: save the table name
 *   - idx 1: ValueListNode: save inserted values
 */
class InsertNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(InsertNode);
    InsertNode(IdentifierNode* tb_name, ValueListNode* value_list)
        : DMLNode(DMLType::kInsert)
    {
        add_child(tb_name);
        add_child(value_list);
    }

    string_t get_tb_name() const {
        IdentifierNode* id_node = dynamic_cast<IdentifierNode*>(at(0));
        assert(id_node);

        return id_node->get_id();
    }

    /** Prepare for tuples because constructing tuples needs Value */
    std::vector<Value> get_values() const {
        std::vector<Value> values;
        ValueListNode* value_list_node = dynamic_cast<ValueListNode*>(at(1));
        assert(value_list_node);

        std::vector<Node*> children = value_list_node->get_values();
        values.resize(children.size());

        size_t size = children.size();
        for (size_t i = 0; i < size; i++) {
            ValueNode* value_node = dynamic_cast<ValueNode*>(children[i]);
            assert(value_node);

            value_node->set_value(&(values[i]));
        }
        
        return values;
    }
};

class SimpleExprNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(SimpleExprNode);
    SimpleExprNode() : DMLNode(DMLType::kSimpleExpr) {}
};

class BitExprNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(BitExprNode);
    BitExprNode() : DMLNode(DMLType::kBitExpr) {}
};

class PredicateNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(PredicateNode);
    PredicateNode() : DMLNode(DMLType::kPredicate) {}
};

class ComparisonOprNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(ComparisonOprNode);
    ComparisonOprNode() : DMLNode(DMLType::kComparisonOpr) {}
};

class BooleanPrimaryNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(BooleanPrimaryNode);
    BooleanPrimaryNode() : DMLNode(DMLType::kBooleanPrimary) {}
};

class ExprNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(ExprNode);
    ExprNode() : DMLNode(DMLType::kExpr) {}
};

class WhereCondNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(WhereCondNode);
    WhereCondNode() : DMLNode(DMLType::kWhereCond) {}
};

class TableFactor : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(TableFactor);
    TableFactor() : DMLNode(DMLType::kTableFactor) {}
};

class TableRefNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(TableRefNode);
    TableRefNode() : DMLNode(DMLType::kTableRef) {}
};

class TableRefsNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(TableRefsNode);
    TableRefsNode() : DMLNode(DMLType::kTableRefs) {}
};

class SelectExprNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(SelectExprNode);
    SelectExprNode() : DMLNode(DMLType::kSelectExpr) {}
};

class SelectExprListNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(SelectExprListNode);
    SelectExprListNode() : DMLNode(DMLType::kSelectExprList) {}
};

class SelectNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(SelectNode);
    SelectNode() : DMLNode(DMLType::kSelect) {

    }

    ExecutorAbstract* get_root_node() const {
        // TODO: Construct the tree from ast
        return nullptr;
    }
};

} // namespace dawn
