#pragma once

#include "executors/executor_abstr.h"
#include "sql/expressions/col_value_expr.h"
#include "table/schema.h"
#include "util/config.h"
#include "data/types.h"

namespace dawn {

template<typename T>
Value init_agg_min_val() {
    return Value(T INT32_MAX);
}

template<typename T>
Value init_agg_max_val() {
    return Value(T INT32_MIN);
}

template<typename T>
Value init_agg_sum_val() {
    return Value(static_cast<T>(0));
}

inline Value init_agg_cnt_val() {
    return Value(static_cast<integer_t>(0));
}

/**
 * For convenience, projection and aggregation function are put together in this ProjectionExecutor.
 * 
 * We put all the aggregation result at the end of the tuple when we need aggregation,
 * so that we can construct a tuple more convenient. output schema should follow this rule
 * when constructing
 */
class ProjectionExecutor : public ExecutorAbstract {
public:
    /**
     * This is a constructor for a pure projection executor without any aggregation operation
     * @param exprs used for projection operation
     */
    ProjectionExecutor(ExecutorContext *exec_ctx, ExecutorAbstract *child, std::vector<ExpressionAbstract*> exprs,
        Schema *input_schema, Schema *output_schema)
        : ExecutorAbstract(exec_ctx), child_(child), exprs_(exprs), input_schema_(input_schema),
         output_schema_(output_schema), is_aggregate_(false) {}

    /**
     * Call this constructor when we need aggregation operation
     * @param exprs used for projection operation
     * @param output_schema consists of two fields: <projection field | aggregation field>, so we always append the
     *                      aggregation result to the end of the projection field.
     * @param agg_type store the type of the aggregation
     * @param agg_exprs store the aggregation expression
     */
    ProjectionExecutor(ExecutorContext *exec_ctx, ExecutorAbstract *child, std::vector<ExpressionAbstract*> exprs,
        std::vector<AggregationType> agg_type, std::vector<ExpressionAbstract*> agg_exprs, Schema *input_schema, Schema *output_schema)
        : ExecutorAbstract(exec_ctx), child_(child), exprs_(exprs), agg_exprs_(agg_exprs), input_schema_(input_schema),
         output_schema_(output_schema), is_aggregate_(true), agg_num_(agg_type.size()) {
        
        /**
         * aggregation field follows up the projection field, so we need to get the length of the
         * projection before we want to locate the aggregation operation's position
         */
        size_t_ proj_col_num = exprs.size();

        // initialize the Value of the aggregation
        for (int i = 0; i < agg_num_; i++) {
            TypeId type_id = output_schema_->get_column_type(proj_col_num + i);
            switch (agg_type[i]) {
                case AggregationType::kCountAggregate:
                    agg_vals_.push_back(init_agg_cnt_val());
                    break;
                case AggregationType::kSumAggregate:
                    if (type_id == TypeId::kInteger) {
                        agg_vals_.push_back(init_agg_sum_val<integer_t>());
                    } else if (type_id == TypeId::kDecimal) {
                        agg_vals_.push_back(init_agg_sum_val<decimal_t>());
                    } else {
                        FATAL("Invalid Aggregation Type");
                    }
                    break;
                case AggregationType::kMinAggregate:
                    if (type_id == TypeId::kInteger) {
                        agg_vals_.push_back(init_agg_min_val<integer_t>());
                    } else if (type_id == TypeId::kDecimal) {
                        agg_vals_.push_back(init_agg_min_val<decimal_t>());
                    } else {
                        FATAL("Invalid Aggregation Type");
                    }
                    break;
                case AggregationType::kMaxAggregate:
                    if (type_id == TypeId::kInteger) {
                        agg_vals_.push_back(init_agg_max_val<integer_t>());
                    } else if (type_id == TypeId::kDecimal) {
                        agg_vals_.push_back(init_agg_max_val<decimal_t>());
                    } else {
                        FATAL("Invalid Aggregation Type");
                    }
                    break;
                default:
                    FATAL("Invalid AggregationType");
            }
        }
    }

    // TODO: Destructor should delete the following members:
    // child_, exprs_, agg_exprs_, input_schema_, output_schema_
    // Maybe type of some members should not be the pointer, such as input_schema_ etc.
    ~ProjectionExecutor() = default;

    DISALLOW_COPY_AND_MOVE(ProjectionExecutor);

    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;
private:
    ExecutorAbstract *child_;
    std::vector<ExpressionAbstract*> exprs_; // get the columns from input tuple
    Tuple next_tuple_; // in avoid of the construction and deconstruction every time we call the get_next()
    Schema *input_schema_; // FIXME: Pointer should be managed by deconstructor
    Schema *output_schema_;

    std::vector<ExpressionAbstract*> agg_exprs_; // aggregate operation
    bool is_aggregate_;
    std::vector<Value> agg_vals_; // store the aggregation result
    int agg_num_;
};

} // namespace dawn