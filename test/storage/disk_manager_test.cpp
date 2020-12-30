#include "gtest/gtest.h"
#include "util/config.h"
#include "util/util.h"
#include "storage/disk/disk_manager.h"

#include <iostream>
#include <fstream>
#include <string>
#include <atomic>

using std::fstream;
using std::string;
using std::ios;

namespace dawn {

/**
 * Test List:
 *   1. create mode should fail when file exists
 *   2. create mode should success when file doesn't exist with "next_page_id == 0"
 *   3. read mode should success when file exists with "next_page_id == xxx"
 *   4. read mode should fail when file doesn't exist
 */
TEST(DiskManagerTest, DISABLED_ConstructorTEST) {
    const char *dbf = "test.db";
    const char *logf = "test.log";
    
    remove(dbf);
    remove(logf);

    fstream db_io;
    fstream log_io;
    string f_name("test");
    
    // test 1
    {
        db_io.open(dbf, ios::out);
        ASSERT_TRUE(db_io.is_open());
        log_io.open(logf, std::ios::out);
        ASSERT_TRUE(log_io.is_open());

        DiskManager dm(f_name);
        ASSERT_FALSE(dm.get_status());

        db_io.close();
        log_io.close();
        db_io.clear();
        log_io.clear();
    }

    // test 2
    {
        remove(dbf);
        remove(logf);

        DiskManager dm(f_name);
        ASSERT_TRUE(dm.get_status());

        db_io.close();
        log_io.close();
        db_io.clear();
        log_io.clear();
    }

    // test 3
    {
        db_io.open(dbf, ios::out);
        ASSERT_TRUE(db_io.is_open());
        log_io.open(logf, ios::out);
        ASSERT_TRUE(log_io.is_open());

        char c = 'c';
        db_io.seekg(40959);
        db_io.write(&c, 1); // write 9 pages into file
        db_io.flush();

        DiskManager dm(f_name, false);
        ASSERT_TRUE(dm.get_status());

        db_io.close();
        log_io.close();
        db_io.clear();
        log_io.clear();
    }

    // test 4
    {
        remove(dbf);
        remove(logf);

        DiskManager dm(f_name, false);
        ASSERT_FALSE(dm.get_status());

        db_io.close();
        log_io.close();
        db_io.clear();
        log_io.clear();
    }
}

TEST(DiskManagerTest, cTest) {
}

} // namespace dawn