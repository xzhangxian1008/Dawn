#pragma once

#include "ast/node.h"

namespace dawn {

enum DDLType : int8_t {
    kSelect,
    kINSERT,
    kDelete
};

class DMLNode : public Node {
public:
    DISALLOW_COPY_AND_MOVE(DMLNode);
    DDLNode(DMLNode type) : type_(type), Node(NodeType::kDDL) {}

    /** We have to dynamic_cast this node in the outside */
    Node* get_node() const { return at(0); }

    DMLNode get_dml_type() const { return type_; }
private:
    DMLNode type_;
};

} // namespace dawn
