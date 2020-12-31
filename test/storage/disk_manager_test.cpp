#include "gtest/gtest.h"
#include "util/config.h"
#include "util/util.h"
#include "storage/disk/disk_manager.h"

#include <iostream>
#include <fstream>
#include <string>
#include <atomic>

using std::ios;

namespace dawn {

class DiskManagerTest : public DiskManager {
public:
    DiskManagerTest(const string_t &meta_name, bool create = false) : DiskManager(meta_name, create) {}

    offset_t get_db_name_sz_offset() const { return db_name_sz_offset; }
    offset_t get_db_name_offset() const { return db_name_offset; }
    offset_t get_log_name_sz_offset() const { return log_name_sz_offset; }
    offset_t get_log_name_offset() const { return log_name_offset; }
    offset_t get_max_ava_pgid_offset() const { return max_ava_pgid_offset; }
    offset_t get_max_alloced_pgid_offset() const { return max_alloced_pgid_offset; }
    offset_t get_reserved_offset() const { return reserved_offset; }
};

/**
 * Test List:
 *   1. create mode: check files have been created and values have been written into the .mtd
 *   2. read mode: read all files successfully and initialize data correctly
 */
TEST(DMTest, DISABLED_ConstructorTEST) {
    const char *mtdf = "test.mtd";
    const char *dbf = "test.db";
    const char *logf = "test.log";

    string_t mtdf_s(mtdf);
    string_t dbf_s(dbf);
    string_t logf_s(logf);
    
    remove(mtdf);
    remove(dbf);
    remove(logf);

    {
        // test 1
        DiskManagerTest dmt("test", true);
        EXPECT_TRUE(dmt.get_status());

        // check files have been created successfully
        EXPECT_FALSE(check_inexistence(mtdf));
        EXPECT_FALSE(check_inexistence(dbf));
        EXPECT_FALSE(check_inexistence(logf));

        // check values have been written into the file
        fstream_t f;
        EXPECT_TRUE(open_file(mtdf, f, ios::in));

        char *buf = new char[512];
        int buf_sz = 512;

        f.seekp(0);
        f.read(buf, buf_sz);
        EXPECT_FALSE(f.fail());
        EXPECT_EQ(f.gcount(), 512);

        int *p;
        p = reinterpret_cast<int*>(buf + dmt.get_db_name_sz_offset());
        EXPECT_EQ(*p, dbf_s.length());

        char *str;
        str = reinterpret_cast<char*>(buf + dmt.get_db_name_offset());
        for (int i = 0; i < dbf_s.length(); i++)
            EXPECT_EQ(dbf_s[i], str[i]);
        EXPECT_EQ('\0', str[dbf_s.length()]);

        p = reinterpret_cast<int*>(buf + dmt.get_log_name_sz_offset());
        EXPECT_EQ(*p, logf_s.length());

        str = reinterpret_cast<char*>(buf + dmt.get_log_name_offset());
        for (int i = 0; i < logf_s.length(); i++)
            EXPECT_EQ(logf_s[i], str[i]);
        EXPECT_EQ('\0', str[logf_s.length()]);
        
        page_id_t *pg;
        pg = reinterpret_cast<page_id_t*>(buf + dmt.get_max_ava_pgid_offset());
        EXPECT_EQ(dmt.get_max_ava_pgid(), *pg);

        pg = reinterpret_cast<page_id_t*>(buf + dmt.get_max_alloced_pgid_offset());
        EXPECT_EQ(dmt.get_max_alloced_pgid(), *pg);

        remove(mtdf);
        remove(dbf);
        remove(logf);
        delete buf;
    }

    {
        // test 2
        
    }
}

TEST(DMTest, cTest) {

}

} // namespace dawn