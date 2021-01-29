#include "buffer/clock_replacer.h"

namespace dawn {

ClockReplacer::ClockReplacer(int pool_size) 
    : pool_size_(pool_size), clock_pointer_(0), exit_num(0) {
    flags = new char[pool_size];
    for (int i = 0; i < pool_size; i++)
        flags[i] = FRAME_NOT_EXIST;
}

void ClockReplacer::pin(frame_id_t frame_id) {
    std::lock_guard<std::mutex> lk(latch_);
    if (flags[frame_id] != FRAME_NOT_EXIST){
        exit_num--;
        flags[frame_id] = FRAME_NOT_EXIST;
    }
}

void ClockReplacer::unpin(frame_id_t frame_id) {
    std::lock_guard<std::mutex> lk(latch_);
    if (flags[frame_id] == FRAME_NOT_EXIST) {
        exit_num++;
        flags[frame_id] = FRAME_EXIST_TRUE;
    }
}

void ClockReplacer::victim(frame_id_t *frame_id) {
    std::unique_lock<std::mutex> lk(latch_, std::defer_lock);
    while (true) {
        lk.lock();
        if (exit_num > 0) {
            break;
        }
        lk.unlock();
        // wait for a frame
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    while (true) {        
        if (flags[clock_pointer_] & FRAME_EXIST_FALSE) {
            *frame_id = clock_pointer_;
            flags[clock_pointer_] = FRAME_NOT_EXIST;
            exit_num--;
            clock_pointer_to_next();
            return;
        }

        if (flags[clock_pointer_] & FRAME_EXIST_TRUE) {
            flags[clock_pointer_] = FRAME_EXIST_FALSE;
            clock_pointer_to_next();
            continue;
        }

        clock_pointer_to_next();
    }
}

int ClockReplacer::size() {
    std::lock_guard<std::mutex> lk(latch_);
    int tmp = exit_num;
    return tmp;
}

} // namespace dawn
