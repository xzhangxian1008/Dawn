#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <thread>
#include <chrono>

#include "storage/page/page.h"
#include "util/util.h"
#include "util/config.h"
#include "data/types.h"
#include "data/values.h"


using std::fstream;
using std::string;
using std::ios;
using std::set;
using std::cout;
using std::endl;
using std::ends;

namespace dawn {

std::mutex mt;
int exist_num = 100;

void thread_print() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    int print_num = 100;

    for (int i = 0; i < print_num; i++) {
        PRINT("I'm", "printing", "messages", "...");
    }
    
    mt.lock();
    exist_num--;
    mt.unlock();
}

TEST(CommonTest1, CommonTEST11) {
    for (int i = 0; i < exist_num; i++) {
        std::thread my_thread(thread_print);
        my_thread.detach();
    }

    while (true) {
        mt.lock();
        if (exist_num == 0) {
            mt.unlock();
            return;
        }
        mt.unlock();
    }
}

} // namespace dawn