#pragma once

#include "ast/node.h"

namespace dawn {

enum DMLType : int8_t {
    kSelect,
    kINSERT,
    kDelete
};

class DMLNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(DMLNode);
    DMLNode(DMLType type) : type_(type), Node(NodeType::kDDL) {}

    /** We have to dynamic_cast this node in the outside */
    Node* get_node() const { return at(0); }

    DMLType get_dml_type() const { return type_; }
private:
    DMLType type_;
};

} // namespace dawn
