#pragma once

#include "buffer/replacer_abstract.h"

#include <chrono>
#include <thread>
#include <mutex>

namespace dawn {

class ClockReplacer : public ReplacerAbstract {
public:
    explicit ClockReplacer(int pool_size);
    ~ClockReplacer() { delete[] flags; }

    void pin(frame_id_t frame_id) override;
    void unpin(frame_id_t frame_id) override;
    void victim(frame_id_t *frame_id) override;
    int size() override;

private:
    // ATTENTION no lock protects this function.
    inline void clock_pointer_to_next() { clock_pointer_ = (clock_pointer_ + 1) % pool_size_; }

    char *flags;
    int pool_size_;
    int clock_pointer_;

    // record how many frames in the replacer
    int exit_num;

    std::mutex latch_;
};
    
} // namespace dawn
