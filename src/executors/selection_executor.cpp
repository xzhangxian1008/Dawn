#include "executors/selection_executor.h"

namespace dawn {

void SelectionExecutor::open() {
    child_->open();
}

bool SelectionExecutor::get_next(Tuple *tuple) {
    while (child_->get_next(tuple)) {
        if (predicate_->evaluate(tuple, schema_) == cmp_) {
            return true;
        }
    }

    return false;
}

void SelectionExecutor::close() {
    child_->close();
}

} // namespace dawn
