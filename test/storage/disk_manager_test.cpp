#include "gtest/gtest.h"
#include "util/config.h"
#include "util/util.h"
#include "storage/disk/disk_manager.h"
#include "storage/page/page.h"

#include <iostream>
#include <fstream>
#include <string>
#include <atomic>
#include <unordered_set>


using std::ios;

namespace dawn {

char buf[1024];
int buf_sz = 1024;

bool create_meta_file(const char *name, page_id_t max_ava_pgid);


class DiskManager_T : public DiskManager {
public:
    DiskManager_T(const string_t &meta_name, bool create = false) : DiskManager(meta_name, create) {}

    offset_t get_db_name_sz_offset() const { return db_name_sz_offset; }
    offset_t get_db_name_offset() const { return db_name_offset; }
    offset_t get_log_name_sz_offset() const { return log_name_sz_offset; }
    offset_t get_log_name_offset() const { return log_name_offset; }
    offset_t get_max_ava_pgid_offset() const { return max_ava_pgid_offset; }
    offset_t get_catalog_pgid_offset() const { return catalog_pgid_offset; }
    offset_t get_reserved_offset() const { return reserved_offset; }
};

class DiskManagerTest : public ::testing::Test {
public:
    const char *meta1 = "read_mode1";
    const char *meta_name1 = "read_mode1.mtd";
    const char *db_name1 = "read_mode1.db";
    const char *log_name1 = "read_mode1.log";
    const page_id_t max_ava_pgid1 = 300;

    void SetUp() override {
        remove(meta_name1);
        remove(db_name1);
        remove(log_name1);
        if (!create_meta_file(meta1, max_ava_pgid1)) {
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

bool create_meta_file(const char *name, page_id_t max_ava_pgid) {
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
        LOG("can't create");
        return false;
    }
    if (!open_file(db_name_, db_io, ios::out)) {
        LOG("can't create");
        return false;
    }
    if (!open_file(log_name_, log_io, ios::out)) {
        LOG("can't create");
        return false;
    }
    offset_t db_name_sz_offset;
    offset_t db_name_offset;
    offset_t log_name_sz_offset;
    offset_t log_name_offset;
    offset_t max_ava_pgid_offset;
    offset_t reserved_offset;
    offset_t catalog_pgid_offset;

    // write meta data to the buffer and set the offset
    int *p;
    db_name_sz_offset = 0;
    p = reinterpret_cast<int*>(buf + db_name_sz_offset);
    *p = db_name_.length();

    db_name_offset = db_name_sz_offset + sizeof(int);
    for (size_t i = 0; i < db_name_.length(); i++)
        buf[db_name_offset+i] = db_name_[i];
    buf[db_name_offset+db_name_.length()] = '\0';

    log_name_sz_offset = db_name_offset + db_name_.length() + 1;
    p = reinterpret_cast<int*>(buf+log_name_sz_offset);
    *p = log_name_.length();

    log_name_offset = log_name_sz_offset + sizeof(int);
    for (size_t i = 0; i < log_name_.length(); i++)
        buf[log_name_offset+i] = log_name_[i];
    buf[log_name_offset+log_name_.length()] = '\0';
    max_ava_pgid_offset = log_name_offset + log_name_.length() + 1;

    page_id_t *pt;
    pt = reinterpret_cast<page_id_t*>(buf+max_ava_pgid_offset);
    *pt = max_ava_pgid;

    catalog_pgid_offset = max_ava_pgid_offset + PGID_T_SIZE;
    *reinterpret_cast<page_id_t*>(buf+catalog_pgid_offset) = 123;

    reserved_offset = catalog_pgid_offset + sizeof(page_id_t);
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

bool read_write_pages_check(DiskManager_T &dmt, int page_num) {
    std::unordered_set<page_id_t> s;
    int num = 100;

    // write something
    for (int i = 0; i < page_num; i++) {
        page_id_t new_pgid = dmt.get_new_page();
        if (new_pgid == INVALID_PAGE_ID)
            return false;
        s.insert(new_pgid);
        Page pg(new_pgid);
        char *data = pg.get_data();
        page_id_t *pt;
        for (int j = 0; j < num; j++) {
            pt = reinterpret_cast<page_id_t*>
                    (data + COM_PG_HEADER_SZ + j * sizeof(page_id_t));
            *pt = new_pgid;
        }
        if (!dmt.write_page(new_pgid, data))
            return false;
    }

    // read
    char read_buf[4096];
    bool ok = true;
    for (auto iter = s.begin(); iter != s.end(); iter++) {
        if (!dmt.read_page(*iter, read_buf))
            return false;

        // check status
        if (*reinterpret_cast<char*>(read_buf) != 1)
            ok = false;
        // check LSN
        if (*reinterpret_cast<lsn_t*>(read_buf + LSN_OFFSET) != -1)
            ok = false;
        // check page id
        if (*reinterpret_cast<page_id_t*>(read_buf + PAGE_ID_OFFSET) != *iter)
            ok = false;

        if (!ok) break;

        // check contents
        for (int i = 0; i < num; i++) {
            if (*iter != *reinterpret_cast<page_id_t*>(read_buf + COM_PG_HEADER_SZ + i * sizeof(page_id_t))) {
                ok = false;
                break;
            }
        }

        if (!ok) break;
    }

    return ok;
}

TEST_F(DiskManagerTest, CasualTest)
{
    const char* meta = "casualTest";
    const char *mtdf = "casualTest.mtd";
    const char *dbf = "casualTest.db";
    const char *logf = "casualTest.log";

    string_t mtdf_s(mtdf);
    string_t dbf_s(dbf);
    string_t logf_s(logf);

    remove(mtdf);
    remove(dbf);
    remove(logf);

    {
        DiskManager_T DmtLyb(meta, true);
        EXPECT_TRUE(DmtLyb.get_status());
    }
}

/**
 * Test List:
 *   1. create mode: check files have been created and values have been written into the .mtd
 *   2. read mode: read all files successfully and initialize data correctly
 *   3. read mode: db should collect page id's info correctly when it starts
 */
TEST_F(DiskManagerTest, ConstructorTEST) {
    const char *meta = "test";
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
        DiskManager_T dmt(meta, true);
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
        for (size_t i = 0; i < dbf_s.length(); i++) {
            EXPECT_EQ(dbf_s[i], str[i]);
        }
        EXPECT_EQ('\0', str[dbf_s.length()]);

        p = reinterpret_cast<int*>(buf + dmt.get_log_name_sz_offset());
        EXPECT_EQ(*p, logf_s.length());

        str = reinterpret_cast<char*>(buf + dmt.get_log_name_offset());
        for (size_t i = 0; i < logf_s.length(); i++)
            EXPECT_EQ(logf_s[i], str[i]);
        EXPECT_EQ('\0', str[logf_s.length()]);
        
        page_id_t *pg;
        pg = reinterpret_cast<page_id_t*>(buf + dmt.get_max_ava_pgid_offset());
        EXPECT_EQ(dmt.get_max_ava_pgid(), *pg);

        pg = reinterpret_cast<page_id_t*>(buf + dmt.get_catalog_pgid_offset());
        EXPECT_EQ(0, *pg);

        char rbuf[PAGE_SIZE];
        EXPECT_TRUE(dmt.read_page(0, rbuf));
        EXPECT_EQ(rbuf[0], STATUS_EXIST);

        delete[] buf;
    }

    remove(mtdf);
    remove(dbf);
    remove(logf);

    {
        // test 2
        DiskManager dm(meta1);
        EXPECT_TRUE(dm.get_status());
        EXPECT_EQ(max_ava_pgid1, dm.get_max_ava_pgid());
        EXPECT_EQ(-1, dm.get_max_alloced_pgid());
        EXPECT_EQ(123, dm.get_catalog_pgid());
    }

    {
        // test 3
        int page_num = 1000;
        {
            // allocate and free some pages
            DiskManager_T dmt(meta, true);
            ASSERT_TRUE(dmt.get_status());

            // check files have been created successfully
            ASSERT_FALSE(check_inexistence(mtdf));
            ASSERT_FALSE(check_inexistence(dbf));
            ASSERT_FALSE(check_inexistence(logf));

            EXPECT_TRUE(read_write_pages_check(dmt, page_num));
            EXPECT_EQ(1000, dmt.get_max_alloced_pgid());

            // free some pages
            for (int i = 1; i < page_num; i++) {
                if (i % 2 == 0)
                    continue;
                ASSERT_TRUE(dmt.free_page(i));
            }
        }

        {   
            // restart and collect information
            DiskManager_T dmt(meta);
            ASSERT_TRUE(dmt.get_status());

            // check files have been created successfully
            ASSERT_FALSE(check_inexistence(mtdf));
            ASSERT_FALSE(check_inexistence(dbf));
            ASSERT_FALSE(check_inexistence(logf));

            // check correctness
            bool ok = true;
            for (int i = 1; i < page_num; i++) {
                if (i % 2 == 0) {
                    if (dmt.is_allocated(i) != true)
                        ok = false;
                    if (dmt.is_free(i) != false)
                        ok = false;
                } else {
                    if (dmt.is_allocated(i) != false)
                        ok = false;
                    if (dmt.is_free(i) != true)
                        ok = false;
                }

                if (!ok) break;
                
            }

            EXPECT_TRUE(ok);
            EXPECT_EQ(1000, dmt.get_max_alloced_pgid());
        }
    }

    remove(mtdf);
    remove(dbf);
    remove(logf);

}

/**
 * Test List:
 *   1. write some pages and read it successfully with correct data
 *   2. write lots of pages and read it successfully with correct data
 *   3. release some pages and check operation have been allpied
 */
TEST_F(DiskManagerTest, DMFunctionTest) {
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
        ASSERT_TRUE(dmt.get_status());

        // check files have been created successfully
        ASSERT_FALSE(check_inexistence(mtdf));
        ASSERT_FALSE(check_inexistence(dbf));
        ASSERT_FALSE(check_inexistence(logf));

        // basic test
        EXPECT_TRUE(read_write_pages_check(dmt, 10));
    }

    remove(mtdf);
    remove(dbf);
    remove(logf);

    {
        // test 2
        DiskManager_T dmt("test", true);
        ASSERT_TRUE(dmt.get_status());

        // check files have been created successfully
        ASSERT_FALSE(check_inexistence(mtdf));
        ASSERT_FALSE(check_inexistence(dbf));
        ASSERT_FALSE(check_inexistence(logf));

        // check lots of pages
        EXPECT_TRUE(read_write_pages_check(dmt, 10000));
    }

    remove(mtdf);
    remove(dbf);
    remove(logf);

    {
        // test 3
        DiskManager_T dmt("test", true);
        ASSERT_TRUE(dmt.get_status());

        // check files have been created successfully
        ASSERT_FALSE(check_inexistence(mtdf));
        ASSERT_FALSE(check_inexistence(dbf));
        ASSERT_FALSE(check_inexistence(logf));

        int page_num = 1000;
        EXPECT_TRUE(read_write_pages_check(dmt, page_num));
        EXPECT_EQ(1000, dmt.get_max_alloced_pgid());

        // free some pages and check the data's correstness
        for (int i = 1; i < page_num; i++) {
            if (i % 2 == 0)
                continue;
            ASSERT_TRUE(dmt.free_page(i));
        }

        // check correctness
        bool ok = true;
        for (int i = 1; i < page_num; i++) {
            if (i % 2 == 0) {
                if (dmt.is_allocated(i) != true)
                    ok = false;
                if (dmt.is_free(i) != false)
                    ok = false;
            } else {
                if (dmt.is_allocated(i) != false)
                    ok = false;
                if (dmt.is_free(i) != true)
                    ok = false;
            }

            if (!ok) break;
        }

        EXPECT_TRUE(ok);
        EXPECT_EQ(1000, dmt.get_max_alloced_pgid());
    }

    remove(mtdf);
    remove(dbf);
    remove(logf);
}

// TODO test concurrency environment
TEST(ConcurrencyDMTest, DISABLED_CDMTest) {

}

} // namespace dawn