#pragma once

#include "ast/node.h"
#include "data/values.h"
#include "executors/executor_abstr.h"

namespace dawn {

enum DMLType : int8_t {
    kSelect, kInsert, kDelete, kValueList, kValue, kConstant, kSelectExprList,
    kSelectExpr, kTableRefs, kTableRef, kTableFactor, kWhereCond, kExpr, kSimpleExpr,
    kBitExpr, kPredicate, kComparisonOpr, kBooleanPrimary
};

enum ExprType : int8_t {
    kPlus, kMinus, kMultiply, kDivide, kGreater, kLess, kGreaterEq, kLessEq, kEqual
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

/**
 * simple_expr
 *     : literal (1)
 *     | identifier (2)
 * 
 * (1): It has only one child. 
 *        - idx 0: LiteralNode
 * (2): It has only one child.
 *        - idx 0: IdentifierNode
 */
class SimpleExprNode : public DMLNode {
public:
    enum SimpleExprType : int8_t { kLiteral, kIdentifier };

    DISALLOW_COPY_AND_MOVE(SimpleExprNode);
    SimpleExprNode(SimpleExprType type) 
        : DMLNode(DMLType::kSimpleExpr), type_(type) {}

    SimpleExprType get_type() const { return type_; }

    // Because it always has only one child, so we return the first child.
    Node* get_child() const { return at(0); }

private:
    SimpleExprType type_;
};

/**
 * bit_expr:
 *   bit_expr '+' bit_expr (1)
 *   bit_expr '-' bit_expr (1)
 *   bit_expr '*' bit_expr (1)
 *   bit_expr '/' bit_expr (1)
 *   simple_expr (2)
 *   ...
 * 
 * (1): It has two children
 *        - idx 0: BitExprNode
 *        - idx 1: BitExprNode
 * (2): It has only one child
 *        - idx 0: SimpleExprNode
 */
class BitExprNode : public DMLNode {
public:
    enum BitExprType : int8_t { kPlus, kMinus, kMultiply, kDivide, kSimpleExpr };

    DISALLOW_COPY_AND_MOVE(BitExprNode);
    BitExprNode(BitExprType type)
        : DMLNode(DMLType::kBitExpr), type_(type) {}

    BitExprType get_type() const { return type_; }
private:
    BitExprType type_;
};

/**
 * predicate:
 *   bit_expr
 *   ...
 * 
 * So far, it has only one child.
 */
class PredicateNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(PredicateNode);
    PredicateNode() : DMLNode(DMLType::kPredicate) {}

    /** Each production rule of predicate has at least one bit_expr */
    BitExprNode* get_the_first_bit() const {
        Node* node = at(0); // Position of the first bit_expr is always at the index 0
        BitExprNode* bit_expr_node = dynamic_cast<BitExprNode*>(node);
        assert(bit_expr_node);

        return bit_expr_node;
    }
};

/**
 * comparison_operator: = | > | < | >= | <= | != | <>
 * It does have any child.
 */
class ComparisonOprNode : public DMLNode {
public:
    enum ComparisonOprType : int8_t { kEqual, kGreater, kLess, kGreaterEq, kLessEq, kNotEqual };

    DISALLOW_COPY_AND_MOVE(ComparisonOprNode);
    ComparisonOprNode(ComparisonOprType type)
        : DMLNode(DMLType::kComparisonOpr), type_(type) {}

    ComparisonOprType get_type() const { return type_; }
private:
    ComparisonOprType type_;
};

/**
 * boolean_primary:
 *   boolean_primary comparison_operator predicate (1)
 *   predicate
 *   ...
 * 
 * (1): It has three children
 *        - idx 0: BooleanPrimaryNode
 *        - idx 1: ComparisonOprNode
 *        - idx 2: PredicateNode
 */
class BooleanPrimaryNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(BooleanPrimaryNode);
    BooleanPrimaryNode()
        : DMLNode(DMLType::kBooleanPrimary), recursive_(false) {}

    /** True, and this node has a child with BooleanPrimaryNode type */
    bool is_recursive() const { return recursive_; }

    void set_recursive(bool recursive) { recursive_ = recursive; }

    /**
     * No matter what production rule it belongs to, this node always
     * has only one PredicateNode or BooleanPrimaryNode or both of them.
     * @return Return nullptr if it doesn't have PredicateNode. So far, we only
     *         implement two production rules and it will not return nullptr.
     */
    PredicateNode* get_predicate() const {
        Node* node;
        PredicateNode* predicate;

        if (!recursive_)
            node = at(0);
        else
            node = at(2);

        predicate = dynamic_cast<PredicateNode*>(node);
        assert(predicate);
        return predicate;
    }

    /**
     * The BooleanPrimaryNode has at most only one BooleanPrimaryNode.
     * @return Return nullptr if it doesn't have BooleanPrimaryNode.
     */
    BooleanPrimaryNode* get_boolean_primary() const {
        if (!recursive_)
            return nullptr;
        
        Node* node = at(0);
        BooleanPrimaryNode* boolean_primary = dynamic_cast<BooleanPrimaryNode*>(node);
        assert(boolean_primary);
        return boolean_primary;
    }

    /** According to the production rules, this ComparisonOprNode is always at the index 1. */
    ComparisonOprNode* get_comparison_opr() const {
        if (!comparison_)
            return nullptr;
        
        Node* node = at(1);
        ComparisonOprNode* comparison_opr = dynamic_cast<ComparisonOprNode*>(node);
        assert(comparison_opr);
        return comparison_opr;
    }

    void set_comparison_opr(bool comparison) { comparison_ = comparison; }
private:
    // Indicate if this BooleanPrimaryNode has a child with BooleanPrimaryNode type
    bool recursive_;

    // Indicate if this BooleanPrimaryNode has a child with ComparisonOprNode type
    bool comparison_;
};

/**
 * expr:
 *   expr AND expr (1)
 *   expr OR expr (1)
 *   boolean_primary
 * (1): It has two childrean
 *        - idx 0: ExprNode
 *        - idx 1: ExprNode
 */
class ExprNode : public DMLNode {
public:
    enum ExprType : int8_t { kOR, kAND, kNOT, kBooleanPrimary };

    DISALLOW_COPY_AND_MOVE(ExprNode);
    ExprNode(ExprType type)
        : DMLNode(DMLType::kExpr), type_(type) {}

    BooleanPrimaryNode* get_boolean_primary() const {
        if (type_ != ExprType::kBooleanPrimary)
            return nullptr;
        
        Node* node = at(0); // Position of the BooleanPrimaryNode is always at index 0.
        BooleanPrimaryNode* boolean_primary = dynamic_cast<BooleanPrimaryNode*>(node);
        assert(boolean_primary);
        return boolean_primary;
    }

    std::vector<ExprNode*> get_exprs() const {
        std::vector<ExprNode*> exprs;
        switch (type_) {
        case ExprType::kBooleanPrimary:
            break;
        case ExprType::kNOT:
            exprs.push_back(get_expr(1));
            break;
        case ExprType::kOR:
        case ExprType::kAND:
            exprs.push_back(get_expr(0));
            exprs.push_back(get_expr(1));
            break;
        default:
            assert(0); // Invalid
        }
        return exprs;
    }

    ExprType get_type() const { return type_; }
private:
    ExprNode* get_expr(size_t idx) const {
        Node* node = at(idx);
        ExprNode* expr = dynamic_cast<ExprNode*>(node);
        assert(expr);
        return expr;
    }

    ExprType type_;
};

using WhereCondNode = ExprNode;
using TableNameNode = IdentifierNode;

/**
 * table_factor:
 *   tb_name
 *   ...
 * 
 * So far, TableFactorNode only provides table name.
 */
class TableFactorNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(TableFactorNode);
    TableFactorNode() : DMLNode(DMLType::kTableFactor) {}

    string_t get_tb_name() const {
        Node* node = at(0);
        TableNameNode* table_name = dynamic_cast<TableNameNode*>(node);
        assert(table_name);
        return table_name->get_id();
    }
};

/**
 * table_reference:
 *   table_factor
 *   joined_table(Not support this so far)
 */
class TableRefNode : public DMLNode {
public:
    enum TableRefType : int8_t { kTableFactor, kJoinTable };
    DISALLOW_COPY_AND_MOVE(TableRefNode);
    TableRefNode() : DMLNode(DMLType::kTableRef) {}
};

class TableRefsNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(TableRefsNode);
    TableRefsNode() : DMLNode(DMLType::kTableRefs) {}

    std::vector<TableRefNode*> get_tb_refs() const {
        std::vector<Node*> children = get_children();
        std::vector<TableRefNode*> refs;

        // Convert to the target type
        for (Node* child : children) {
            TableRefNode* ref = dynamic_cast<TableRefNode*>(child);
            assert(ref);
            refs.push_back(ref);
        }
        return refs;
    }
};

/**
 * select_expr:
 *   col_name
 *   *
 */
class SelectExprNode : public DMLNode {
public:
    // More functions will be added
    enum SelectExprType : int32_t { kColName, kStar };

    DISALLOW_COPY_AND_MOVE(SelectExprNode);
    SelectExprNode(SelectExprType type)
        : DMLNode(DMLType::kSelectExpr), type_(type) {}
    
    SelectExprType get_type() const { return type_; }

    string_t get_col_name() const {
        if (type_ != SelectExprType::kColName)
            return "";
        
        Node* node = at(0); // Each SelectExprNode has only one child
        IdentifierNode* col_name = dynamic_cast<IdentifierNode*>(node);
        assert(col_name);
        return col_name->get_id();
    }
private:
    SelectExprType type_;
};

class SelectExprListNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(SelectExprListNode);
    SelectExprListNode()
        : DMLNode(DMLType::kSelectExprList), is_star_(false) {}

    bool is_star() const { return is_star_; }
    void set_star(bool star) { is_star_ = star; }

    /** Empty vector will be returned when there is a '*' in the expression list */
    std::vector<string_t> get_col_names() const {
        if (is_star_)
            return std::vector<string_t>{};
        
        std::vector<string_t> col_names;
        std::vector<Node*> nodes = get_children();
        for (Node* child : nodes) {
            SelectExprNode* select_expr = dynamic_cast<SelectExprNode*>(child);
            assert(select_expr);

            // Ignore some expression that is not kColName
            if (select_expr->get_type() != SelectExprNode::SelectExprType::kColName)
                continue;
            col_names.push_back(select_expr->get_col_name());
        }
        return col_names;
    }
private:
    bool is_star_; // True when '*' appear in the select expression list
};

class SelectNode : public DMLNode {
public:
    DISALLOW_COPY_AND_MOVE(SelectNode);
    SelectNode() : DMLNode(DMLType::kSelect) {}

    void set_select_expr_list(SelectExprListNode* select_expr_list) {
        select_expr_list_ = select_expr_list;
    }
    void set_table_refs(TableRefsNode* tb_refs) { tb_refs_ = tb_refs; }
    void set_where_cond(WhereCondNode* where_cond) { where_cond_ = where_cond; }

    SelectExprListNode* get_select_expr_list() const { return select_expr_list_; }
    TableRefsNode* get_table_refs() const { return tb_refs_; }
    WhereCondNode* get_where_cond() const { return where_cond_; }

    ExecutorAbstract* get_root_node() const {
        // TODO: Construct the tree from ast
        return nullptr;
    }
private:
    SelectExprListNode* select_expr_list_;
    TableRefsNode* tb_refs_;
    WhereCondNode* where_cond_;
};

} // namespace dawn
