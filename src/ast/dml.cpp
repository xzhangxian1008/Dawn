#include "ast/dml.h"

namespace dawn {

// TODO: Construct the executor tree from ast
ExecutorAbstract* SelectNode::construct() const {

}

ProjectionExecutor* SelectExprListNode::get_projection(
                        const string_t& tb_name, ExecutorContext *exec_ctx,
                        std::vector<ExpressionAbstract*> exprs,
                        ExecutorAbstract *child, Schema *input_schema) const
{
    if (!check_projection_need())
        return nullptr; // No projection should be done
    

}

std::vector<ExpressionAbstract*> SelectExprListNode::build_exprs(
    const string_t& tb_name, Schema *input_schema) const;
{

}

std::vector<string_t> SelectExprListNode::get_projected_col(const string_t& tb_name) const {
    // TODO: Distinguish different tables' columns
    
}

}

} // namespace dawn
