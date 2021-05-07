#include <thread>
#include <chrono>
#include <random>

#include "gtest/gtest.h"
#include "util/work_pool.h"

namespace dawn {

TEST(WorkPoolTests, BasicTest) {
    WorkPool wp(3);
    wp.start_up();

    int var1 = 111;
    int var2 = 222;
    int var3 = 333;
    wp.add_task([&] {
        for (int i = 0; i < 10000; i++) {
            var1++;
        }
    });
    wp.add_task([&] {
        for (int i = 0; i < 10000; i++) {
            var2--;
        }
    });
    wp.add_task([&] {
        var3 *= var3;
    });

    wp.wait_until_all_finished();
    EXPECT_EQ(var1, 111 + 10000);
    EXPECT_EQ(var2, 222 - 10000);
    EXPECT_EQ(var3, 333 * 333);
    wp.shutdown();
}

TEST(WorkPoolTests, ManyWorkLoadTest) {
    std::mutex mt;
    int count = 0;
    uint32_t work_num = 1000;
    uint32_t thd_num = 50;
    WorkPool wp(thd_num);
    wp.start_up();

    std::default_random_engine dre;
    std::uniform_int_distribution<int> di(5, 200);
    
    for (uint32_t i = 0; i < work_num; i++) {
        wp.add_task([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(di(dre)));
            std::lock_guard lg(mt);
            count++;
        });
    }

    wp.wait_until_all_finished();
    wp.shutdown();
    EXPECT_EQ(count, work_num);
}

} // namespace dawn

