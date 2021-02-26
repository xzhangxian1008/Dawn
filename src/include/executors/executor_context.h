#pragma once

namespace dawn {

class BufferPoolManager;

class ExecutorContext {
public:
    ExecutorContext(BufferPoolManager *bpm) : bpm_(bpm) {}

    ~ExecutorContext() = default;

    BufferPoolManager* get_buffer_pool_manager() const { return bpm_; }
private:
    /** some more context will be added, such as transaction, catalog, etc. */
    BufferPoolManager *bpm_;
};

} // namespace dawn
