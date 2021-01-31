#include "gtest/gtest.h"
#include "util/config.h"
#include "util/util.h"
#include "buffer/clock_replacer.h"

#include <unordered_set>
#include <chrono>
#include <thread>

namespace dawn {

/**
 * Test List:
 *   1. unpin some frames and evict them
 *   2. unpin some frames, pin some of them and evict the rest
 *   3. evict frame while there is no available frame, wait, and
 *      evict successfully after unpin a frame
 */
TEST(ClockReplacer, OneThreadTest) {
    const int POOL_SIZE = 1000;
    
    {
        // test 1
        ClockReplacer cr(POOL_SIZE);

        // unpin some pages and record them
        std::unordered_set<frame_id_t> frames;
        for (frame_id_t i = 0; i < 1000; i += 4) {
            cr.unpin(i);
            frames.insert(i);
        }

        // evict all of them
        int size = frames.size();
        frame_id_t vict = -1;
        for (int i = 0; i < size; i++) {
            cr.victim(&vict);
            auto iter = frames.find(vict);
            EXPECT_NE(iter, frames.end());
            frames.erase(iter);
        }

        EXPECT_EQ(0, frames.size());
        EXPECT_EQ(0, cr.size());
    }

    {
        // test 2
        ClockReplacer cr(POOL_SIZE);

        // unpin some pages and record them
        std::unordered_set<frame_id_t> frames;
        for (frame_id_t i = 0; i < 1000; i += 4) {
            cr.unpin(i);
            frames.insert(i);
        }

        // pin some of them
        int size = frames.size();
        auto iter = frames.begin();
        for (int i = 0; i < size/2; i++) {
            cr.pin(*iter); // when a frame has been pinned, it should be removed from frames.
            auto tmp_iter = iter;
            iter++;
            frames.erase(tmp_iter);
        }

        size = frames.size();
        EXPECT_EQ(size, cr.size());

        // evict the rest
        frame_id_t vict = -1;
        for (int i = 0; i < size; i++) {
            cr.victim(&vict);
            auto iter = frames.find(vict);
            EXPECT_NE(iter, frames.end());
            frames.erase(iter);
        }

        EXPECT_EQ(0, frames.size());
        EXPECT_EQ(0, cr.size());
    }

    {
        // test 3
        ClockReplacer cr(POOL_SIZE);
        frame_id_t id = 111;

        auto func = [&] () {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            cr.unpin(id);
        };

        std::thread thd(func);

        frame_id_t vict_id;
        cr.victim(&vict_id);
        EXPECT_EQ(vict_id, id);

        thd.join();
    }
}

} // namespace dawn
