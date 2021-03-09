#include "executors/selection_executor.h"

namespace dawn {

void SelectionExecutor::open() {
    child_->open();
}

// TODO implement the value vs value (values are in the same tuple)
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
