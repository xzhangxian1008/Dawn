#pragma once

#include <vector>
#include <stdint.h>

#include "util/util.h"
#include "data/types.h"

namespace dawn {

enum class NodeType : int8_t {
    kDDL,
    kDML,
    kIdentity,
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
private:
    std::vector<Node*> children_;
    NodeType type_;
};

class IdentityNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(IdentityNode);
    IdentityNode(std::string id) : id_(id), Node(NodeType::kIdentity) {}

    std::string get_id() const { return id_; }
private:
    std::string id_;
};

class DataTypeNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(DataTypeNode);
    DataTypeNode(TypeId type) : type_(type), char_len_(-1), Node(NodeType::kDataType) {}
    DataTypeNode(TypeId type, int char_len)
        : type_(type), char_len_(char_len), Node(NodeType::kDataType) {}
    
    TypeId get_data_type() const { return type_; }
    int get_char_len() const { return char_len_; }
private:
    TypeId type_;
    int char_len_;
};

class StmtListNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(StmtListNode);
    StmtListNode() : Node(NodeType::kStmtList) {}

    std::vector<StmtNode*> get_stmt_nodes() const {
        return get_children();
    }
};

using StmtNode = Node;

} // namespace dawn
