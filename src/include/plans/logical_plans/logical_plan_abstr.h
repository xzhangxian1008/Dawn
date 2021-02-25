#pragma once

#include <vector>

namespace dawn {

class Schema;

class LogicalPlanAbstr {
public:
    LogicalPlanAbstr(const Schema *output_schema, std::vector<const LogicalPlanAbstr*> &&children)
        : output_schema_(output_schema), children_(std::move(children)) {}
    ~LogicalPlanAbstr() = default;
    const Schema *get_output_schema() const { return output_schema_; }
    std::vector<const LogicalPlanAbstr*> get_children() const { return children_; }
private:
    const Schema *output_schema_;
    std::vector<const LogicalPlanAbstr*> children_;
};

} // namespace dawn
