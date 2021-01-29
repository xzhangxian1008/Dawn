#include <unordered_set>
#include <unordered_map>
#include "gtest/gtest.h"

#include "util/config.h"
#include "util/util.h"
#include "buffer/buffer_pool_manager.h"
#include "storage/disk/disk_manager.h"

namespace dawn {

const int POOL_SIZE = 20;
const char *meta = "bpm_test";
const char *mtdf = "bpm_test.mtd";
const char *dbf = "bpm_test.db";
const char *logf = "bpm_test.log";
string_t mtdf_s(mtdf);
string_t dbf_s(dbf);
string_t logf_s(logf);

template<typename T>
void write_num_to_char(T num, char *dst) {
    string_t num_str = std::to_string(num);
    for (size_t i = 0; i < num_str.length(); i++)
        dst[i] = num_str[i];
    dst[num_str.length()] = '\0';
}

class BPBasicTest : public testing::Test {
public:
    void SetUp() {
        remove(mtdf);
        remove(dbf);
        remove(logf);
    }

    void TearDown() {
        remove(mtdf);
        remove(dbf);
        remove(logf);
    }
};

// ATTENTION encapsulate buffer pool manager's functions, thus it will be modified in the future
class BufferPoolManagerTest : public BufferPoolManager {
public:
    explicit BufferPoolManagerTest(DiskManager *disk_manager, int pool_size)
        : BufferPoolManager(disk_manager, pool_size) {}
    
    Page* get_page_test(page_id_t page_id) { return get_page(page_id); }
    Page* new_page_test() { return new_page(); }
    bool delete_page_test(page_id_t page_id) { return delete_page(page_id); }
    void unpin_page_test(page_id_t page_id, bool is_dirty) { unpin_page(page_id, is_dirty); }
    bool flush_page_test(page_id_t page_id) { return flush_page(page_id); }
    bool flush_all() { return flush_all(); }
    size_t_ get_size() { return BufferPoolManager::get_size(); }

    /**
     * ATTENTION USE THIS FUNCTION CAREFULLY!
     * this ignores the pin count and will cause disasters 
     * with the use of replacer's victim
     * DO NOT USE THIS FUNCTION as possible as you can
     */
    void evict_page_test(page_id_t page_id) { evict_page(page_id, get_frame_id(page_id)); }

    void write_page(Page *page, const offset_t &offset, const char *src, const int size);
    void read_page(Page *page, const offset_t &offset, char *dst, const int &size);

    bool is_in_bpm(const page_id_t &page_id) {
        latch_.r_lock();
        frame_id_t frame_id = get_frame_id(page_id);
        latch_.r_unlock();
        return (frame_id != -1) ? true : false;
    }
};

inline void BufferPoolManagerTest::
write_page(Page *page, const offset_t &offset, const char *src, const int size) {
    page->w_lock();
    memcpy(page->get_data() + offset, src, size);
    page->w_unlock();
}

inline void BufferPoolManagerTest::
read_page(Page *page, const offset_t &offset, char *dst, const int &size) {
    page->r_lock();
    memcpy(dst, page->get_data() + offset, size);
    page->r_unlock();
}

/**
 * Test list:
 *   1. new, write, evict, get, read, write, flush, read with DiskManager, 
 *      check and delete page
 *   2. first phase: store pages in the bpm, write data and unpin some of them
 *      second phase: push a large number of pages into bpm and unpin them
 *      third phase: check if pinned pages pushed in the first phase are in the bpm
 *                   and unpinned pages pushed in the first phase are not in it and 
 *                   their written data have been persisted on the disk.
 *   3. first phase: get large number of pages, write, unpin and flush them
 *      second phase: get them from bpm and check the content has been written
 */
TEST(BPBasicTest, Test1) {
    DiskManager *dm = DiskManagerFactory::create_DiskManager(meta, true);
    ASSERT_NE(dm, nullptr);

    BufferPoolManagerTest bpmt(dm, POOL_SIZE);

    // new
    Page *page = bpmt.new_page_test();
    ASSERT_NE(page, nullptr);
    page_id_t page_id = page->get_page_id();
    ASSERT_TRUE(bpmt.is_in_bpm(page_id));
    ASSERT_FALSE(dm->is_free(page_id));
    ASSERT_TRUE(dm->is_allocated(page_id));

    // write
    const int size1 = 6;
    const char *s1 = "12345";
    bpmt.write_page(page, COM_PG_HEADER_SZ, s1, size1);

    // evict
    bpmt.evict_page_test(page->get_page_id());
    ASSERT_FALSE(bpmt.is_in_bpm(page_id));

    // get
    page = bpmt.get_page_test(page_id);
    ASSERT_TRUE(bpmt.is_in_bpm(page_id));

    // read
    char buf[10];
    bool ok = true;
    bpmt.read_page(page, COM_PG_HEADER_SZ, buf, size1);
    for (int i = 0; i < size1; i++) {
        if (s1[i] != buf[i]) {
            ok = false;
            break;
        }
    }
    EXPECT_TRUE(ok);

    // write
    const int size2 = 9;
    const char *s2 = "abcdefgh";
    bpmt.write_page(page, COM_PG_HEADER_SZ, s2, size2);
    bpmt.unpin_page_test(page->get_page_id(), true);

    // flush
    ASSERT_TRUE(bpmt.flush_page_test(page->get_page_id()));

    // read with DiskManager
    page = bpmt.get_page_test(page_id);
    char page_buf[PAGE_SIZE];
    ASSERT_TRUE(dm->read_page(page->get_page_id(), page_buf));
    bpmt.unpin_page_test(page->get_page_id(), false);

    // check
    ok = true;
    for (int i = 0; i < size2; i++) {
        if (page_buf[i+COM_PG_HEADER_SZ] != s2[i]) {
            ok = false;
            break;
        }
    }
    EXPECT_TRUE(ok);

    // delete page
    page_id = page->get_page_id();
    ASSERT_TRUE(bpmt.delete_page_test(page_id));
    EXPECT_FALSE(bpmt.is_in_bpm(page_id));
    EXPECT_TRUE(dm->is_free(page_id));
    EXPECT_FALSE(dm->is_allocated(page_id));

    delete dm;
}

TEST(BPBasicTest, Test2) {
    DiskManager *dm = DiskManagerFactory::create_DiskManager(meta, true);
    ASSERT_NE(dm, nullptr);

    BufferPoolManagerTest bpmt(dm, POOL_SIZE);

    std::unordered_set<page_id_t> pinned;
    std::unordered_set<page_id_t> unpinned;
    std::unordered_map<page_id_t, Page*> unpinned_pages;

    size_t_ data_size = 20;
    char data[data_size];

    // phase 1
    for (int i = 0; i < POOL_SIZE; i++) {
        memset(data, 0, data_size);
        Page *p = bpmt.new_page_test();
        page_id_t pgid = p->get_page_id();
        if (pgid % 2 == 0) {
            // unpin
            unpinned.insert(pgid);
            unpinned_pages.insert(std::make_pair(pgid, p));
            write_num_to_char(pgid, data);
            bpmt.write_page(p, COM_PG_HEADER_SZ, data, strlen(data)+1);
            bpmt.unpin_page_test(pgid, true);
            continue;
        }
        pinned.insert(pgid);
    }

    // phase 2
    for (int i = 0; i < 5 * POOL_SIZE; i++) {
        Page *p = bpmt.new_page_test();
        bpmt.unpin_page_test(p->get_page_id(), false);
    }

    // phase 3
    bool ok = true;
    for (auto &pgid : pinned) {
        if (!bpmt.is_in_bpm(pgid)) {
            ok = false;
            break;
        }
    }

    EXPECT_TRUE(ok);

    ok = true;
    char page_buf[PAGE_SIZE];
    for (auto &pgid : unpinned) {
        if (bpmt.is_in_bpm(pgid)) {
            ok = false;
            break;
        }

        // directly read page's written data from diskmanager
        ASSERT_TRUE(dm->read_page(pgid, page_buf));
        string_t pgid_str = std::to_string(pgid);

        // check data
        for (size_t i = 0; i < pgid_str.length(); i++) {
            if (page_buf[COM_PG_HEADER_SZ + i] != pgid_str[i]) {
                ok = false;
                break;
            }
        }
        if (page_buf[COM_PG_HEADER_SZ + pgid_str.length()] != '\0')
            ok = false;
        if (!ok)
            break;
    }

    EXPECT_TRUE(ok);

    delete dm;
}

TEST(BPBasicTest, Test3) {
    DiskManager *dm = DiskManagerFactory::create_DiskManager(meta, true);
    ASSERT_NE(dm, nullptr);

    BufferPoolManagerTest bpmt(dm, POOL_SIZE);

    size_t_ data_size = 50;
    char data[data_size];
    std::set<page_id_t> page_ids;

    // phase 1
    for (int i = 0; i < 5 * POOL_SIZE; i++) {
        memset(data, 0, data_size);
        Page *p = bpmt.new_page_test();
        page_id_t pgid = p->get_page_id();
        page_ids.insert(pgid);
        write_num_to_char(pgid, data);
        bpmt.write_page(p, COM_PG_HEADER_SZ, data, strlen(data)+1);
        bpmt.unpin_page_test(pgid, true);
        bpmt.flush_page_test(pgid);
    }


    // phase 2
    bool ok = true;
    for (auto id : page_ids) {
        Page *p = bpmt.get_page_test(id);
        if (p == nullptr) {
            ok = false;
            break;
        }
        string_t pgid_str = std::to_string(id);

        // check data
        for (size_t i = 0; i < pgid_str.length(); i++) {
            if (p->get_data()[COM_PG_HEADER_SZ + i] != pgid_str[i]) {
                ok = false;
                break;
            }
        }
        if (p->get_data()[COM_PG_HEADER_SZ + pgid_str.length()] != '\0')
            ok = false;
        if (!ok)
            break;
        bpmt.unpin_page_test(id, false);
    }
    EXPECT_TRUE(ok);

    delete dm;
}

} // namespace dawn
