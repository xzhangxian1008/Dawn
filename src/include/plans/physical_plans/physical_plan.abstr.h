#pragma once

#include <vector>

namespace dawn {

class Schema;

class PhysicalPlanAbstr {
public:
    PhysicalPlanAbstr(const Schema *output_schema, std::vector<const PhysicalPlanAbstr*> &&children)
        : output_schema_(output_schema), children_(std::move(children)) {}
    ~PhysicalPlanAbstr() = default;
    const Schema *get_output_schema() const { return output_schema_; }
    std::vector<const PhysicalPlanAbstr*> get_children() const { return children_; }
private:
    const Schema *output_schema_;
    std::vector<const PhysicalPlanAbstr*> children_;
};

} // namespace dawn
