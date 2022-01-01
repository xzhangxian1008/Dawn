#pragma once

#include "util/util.h"
#include <vector>
#include <stdint.h>

namespace dawn {

enum class NodeType : int8_t {
    kDDL,
    kDML,
    kIdentity,
    kDataType
};

enum class DataType : int8_t {
    kChar,
    kInteger,
    kBoolean,
    kDecimal
};

/**
 * All nodes related to ast should inherit this class
 */
class Node {
public:
    DISALLOW_COPY_AND_MOVE(Node);
    Node(NodeType type) : type_(type) {}

    ~Node() {
        for(auto node : children_)
            delete node;
    }

    void add_child(const Node* child) { children_.push_back(child); }
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
    DataTypeNode(DataType type) : type_(type), char_len_(-1), Node(NodeType::kDataType) {}
    DataTypeNode(DataType type, int char_len)
        : type_(type), char_len_(char_len), Node(NodeType::kDataType) {}
    
    DataType get_data_type() const { return type_; }
    int get_char_len() const { return char_len_; }
private:
    DataType type_;
    int char_len_;
};

} // namespace dawn
