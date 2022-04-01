#pragma once

#include <vector>
#include <stdint.h>

#include "util/util.h"
#include "data/types.h"

namespace dawn {

enum class NodeType : int8_t {
    kDDL = 0,
    kDML,
    kIdentifier,
    kDataType,
    kStmtList,
    kEmptyStmt
};

/**
 * All nodes related to ast should inherit this class
 */
class Node {
public:
    DISALLOW_COPY_AND_MOVE(Node);
    Node(NodeType type) : type_(type) {}

    virtual ~Node() {
        for(auto node : children_)
            delete node;
    }

    void add_child(Node* child) { children_.push_back(child); }
    NodeType get_type() const { return type_; }
    std::vector<Node*> get_children() const { return children_; }
    Node* at(size_t idx) const { return children_[idx]; }
    size_t get_child_num() const { return children_.size(); }
protected:
    std::vector<Node*> children_;
    NodeType type_;
};

class IdentifierNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(IdentifierNode);
    IdentifierNode(std::string id) : id_(id), Node(NodeType::kIdentifier) {}

    std::string get_id() const { return id_; }
private:
    std::string id_;
};

class DataTypeNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(DataTypeNode);
    DataTypeNode(TypeId type) : type_(type), char_len_(-1), Node(NodeType::kDataType) {}
    DataTypeNode(TypeId type, size_t_ char_len)
        : type_(type), char_len_(char_len), Node(NodeType::kDataType) {}
    
    TypeId get_data_type() const { return type_; }
    size_t_ get_char_len() const { return char_len_; }
private:
    TypeId type_;
    size_t_ char_len_;
};

using StmtNode = Node;

class StmtListNode : public Node {
public:
    DISALLOW_COPY(StmtListNode);
    StmtListNode() : Node(NodeType::kStmtList) {}

    ~StmtListNode() override {}

    StmtListNode& operator=(StmtListNode&& node) {
        this->err_ = node.err_;
        this->type_ = node.type_;
        this->children_ = std::move(node.children_);
        return *this;
    }

    std::vector<StmtNode*> get_stmt_nodes() const {
        return get_children();
    }

    void set_error() { err_ = true; }
    bool is_error() const { return err_; }
private:
    bool err_ = false; // Set to true when error happens.
};

} // namespace dawn
