#include "ast/dml.h"

namespace dawn {

// TODO: Construct the executor tree from ast
ExecutorAbstract* SelectNode::construct() const {

}

ProjectionExecutor* SelectExprListNode::get_projection(
                        ExecutorContext *exec_ctx,
                        ExecutorAbstract *child,
                        Schema *input_schema) const {
    if (!check_projection_need())
        return nullptr; // No projection should be done
    

}

std::vector<ExpressionAbstract*>
SelectExprListNode::build_exprs(Schema *input_schema) const {
    std::vector<string_t> proj_col_names = get_col_names();
    std::vector<ExpressionAbstract*> exprs;
    exprs.reserve(proj_col_names.size());

    for (auto name : proj_col_names) {
        offset_t idx = input_schema->get_column_idx(name);
        exprs.push_back(new ColumnValueExpression(idx));
    }

    return exprs;
}

Schema* SelectExprListNode::build_output_schema() const {
    
}

} // namespace dawn
