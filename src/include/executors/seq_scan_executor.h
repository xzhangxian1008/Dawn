#pragma once

#include "executors/executor_abstr.h"
#include "table/table.h"
#include "table/lk_ha_tb_iter.h"
#include "util/config.h"

namespace dawn {

class SeqScanExecutor : public ExecutorAbstract {
public:
    SeqScanExecutor(ExecutorContext *exec_ctx, Table *table) 
        : ExecutorAbstract(exec_ctx), table_(table), tb_iter_(nullptr) {}

    virtual ~SeqScanExecutor() {
        if (tb_iter_ != nullptr)
            delete tb_iter_;
    }

    DISALLOW_COPY_AND_MOVE(SeqScanExecutor);

    void open() override;
    bool get_next(Tuple *tuple) override;
    void close() override;
private:
    Table *table_;
    TableIterAbstract *tb_iter_;
};

} // namespace dawn
