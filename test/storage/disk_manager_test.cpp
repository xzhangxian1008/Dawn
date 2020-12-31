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

char buf[1024];
int buf_sz = 1024;

bool create_meta_file(const char *name, page_id_t max_ava_pgid, page_id_t max_alloced_pgid) {
    string_t meta_name(name);
    meta_name += ".mtd";
    string_t db_name_(name);
    db_name_ += ".db";
    string_t log_name_(name);
    log_name_ += ".log";

    fstream_t meta_io;
    fstream_t db_io;
    fstream_t log_io;
    if (!open_file(meta_name, meta_io, ios::in | ios::out | ios::trunc)) {
        LOG("can't open");
        return false;
    }
    if (!open_file(db_name_, db_io, ios::out)) {
        LOG("can't open");
        return false;
    }
    if (!open_file(log_name_, log_io, ios::out)) {
        LOG("can't open");
        return false;
    }
    offset_t db_name_sz_offset;
    offset_t db_name_offset;
    offset_t log_name_sz_offset;
    offset_t log_name_offset;
    offset_t max_ava_pgid_offset;
    offset_t max_alloced_pgid_offset;
    offset_t reserved_offset;

    // write meta data to the buffer and set the offset
    int *p;
    db_name_sz_offset = 0;
    p = reinterpret_cast<int*>(buf + db_name_sz_offset);
    *p = db_name_.length();

    db_name_offset = db_name_sz_offset + sizeof(int);
    for (int i = 0; i < db_name_.length(); i++)
        buf[db_name_offset+i] = db_name_[i];
    buf[db_name_offset+db_name_.length()] = '\0';

    log_name_sz_offset = db_name_offset + db_name_.length() + 1;
    p = reinterpret_cast<int*>(buf+log_name_sz_offset);
    *p = log_name_.length();

    log_name_offset = log_name_sz_offset + sizeof(int);
    for (int i = 0; i < log_name_.length(); i++)
        buf[log_name_offset+i] = log_name_[i];
    buf[log_name_offset+log_name_.length()] = '\0';
    max_ava_pgid_offset = log_name_offset + log_name_.length() + 1;

    page_id_t *pt;
    pt = reinterpret_cast<page_id_t*>(buf+max_ava_pgid_offset);
    *pt = max_ava_pgid;

    max_alloced_pgid_offset = max_ava_pgid_offset + sizeof(page_id_t);
    pt = reinterpret_cast<page_id_t*>(buf+max_alloced_pgid_offset);
    *pt = max_alloced_pgid;

    reserved_offset = max_alloced_pgid_offset + sizeof(page_id_t);
    memset(buf + reserved_offset, 0, 128);

    // write meta data to the meta file
    meta_io.seekg(0);
    meta_io.write(buf, 512);
    if (meta_io.fail()) {
        LOG("Write to meta data file fail.");
        return false;
    }
    meta_io.flush();
    return true;
}

class DiskManager_T : public DiskManager {
public:
    DiskManager_T(const string_t &meta_name, bool create = false) : DiskManager(meta_name, create) {}

    offset_t get_db_name_sz_offset() const { return db_name_sz_offset; }
    offset_t get_db_name_offset() const { return db_name_offset; }
    offset_t get_log_name_sz_offset() const { return log_name_sz_offset; }
    offset_t get_log_name_offset() const { return log_name_offset; }
    offset_t get_max_ava_pgid_offset() const { return max_ava_pgid_offset; }
    offset_t get_max_alloced_pgid_offset() const { return max_alloced_pgid_offset; }
    offset_t get_reserved_offset() const { return reserved_offset; }
};

class DiskManagerTest : public ::testing::Test {
public:
    const char *meta1 = "read_mode";
    const char *meta_name1 = "read_mode.mtd";
    const char *db_name1 = "read_mode.db";
    const char *log_name1 = "read_mode.log";
    const page_id_t max_ava_pgid1 = 300;
    const page_id_t max_alloced_pgid1 = 100;

    void SetUp() override {
        remove(meta_name1);
        remove(db_name1);
        remove(log_name1);
        if (!create_meta_file(meta1, max_ava_pgid1, max_alloced_pgid1)) {
            LOG("create file fail");
            exit(-1);
        }
    }

    void TearDown() override {
        remove(meta_name1);
        remove(db_name1);
        remove(log_name1);
    }
};

/**
 * Test List:
 *   1. create mode: check files have been created and values have been written into the .mtd
 *   2. read mode: read all files successfully and initialize data correctly
 *   TODO other test
 *   3. read mode: based on the test 2, we should collect page id's info correctly
 */
TEST_F(DiskManagerTest, ConstructorTEST) {
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
        DiskManager_T dmt("test", true);
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
        DiskManager dm(meta1);
        EXPECT_TRUE(dm.get_status());
        EXPECT_EQ(max_ava_pgid1, dm.get_max_ava_pgid());
        EXPECT_EQ(max_alloced_pgid1, dm.get_max_alloced_pgid());
    }

    {
        // TODO test 3
    }
}

TEST(DMTest, DISABLED_DMFunctionTest) {

}

} // namespace dawn