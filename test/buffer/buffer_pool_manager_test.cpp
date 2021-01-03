#include "gtest/gtest.h"

#include "util/config.h"
#include "util/util.h"
#include "buffer/buffer_pool_manager.h"
#include "storage/disk/disk_manager.h"

namespace dawn {

// ATTENTION encapsulate buffer pool manager's functions, thus it will be modified in the future
class BufferPoolManagerTest : public BufferPoolManager {
public:
    explicit BufferPoolManagerTest(DiskManager *disk_manager, int pool_size)
        : BufferPoolManager(disk_manager, pool_size) {}
    
    Page* get_page_test(page_id_t page_id) { return get_page(page_id); }
    Page* new_page_test() { return new_page(); }
    bool delete_page_test(page_id_t page_id) { return delete_page(page_id); }
    void unpin_page_test(page_id_t page_id) { unpin_page(page_id); }
    bool flush_page_test(page_id_t page_id) { return flush_page(page_id); }

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

void BufferPoolManagerTest::
write_page(Page *page, const offset_t &offset, const char *src, const int size) {
    // write page
    page->w_lock();
    memcpy(page->get_data() + offset, src, size);

    // set dirty
    page->set_is_dirty(true);
    page->w_unlock();
}

void BufferPoolManagerTest::
read_page(Page *page, const offset_t &offset, char *dst, const int &size) {
    page->r_lock();
    memcpy(dst, page->get_data() + offset, size);
    page->r_unlock();
}

/**
 * Test list:
 *   1. new, write, evict, get, read, write, flush, read with DiskManager, 
 *      check and delete page
 */
TEST(BPMTest, BasicTest) {
    const int POOL_SIZE = 5;
    const char *meta = "bpm_test";
    const char *mtdf = "bpm_test.mtd";
    const char *dbf = "bpm_test.db";
    string_t mtdf_s(mtdf);
    string_t dbf_s(dbf);
    remove(mtdf);
    remove(dbf);

    {
        // test 1
        DiskManager dm(meta, true);
        ASSERT_TRUE(dm.get_status());
        ASSERT_FALSE(check_inexistence(mtdf));
        ASSERT_FALSE(check_inexistence(dbf));

        BufferPoolManagerTest bpmt(&dm, POOL_SIZE);

        // new
        Page *page = bpmt.new_page_test();
        page_id_t page_id = page->get_page_id();
        ASSERT_TRUE(bpmt.is_in_bpm(page_id));
        ASSERT_FALSE(dm.is_free(page_id));
        ASSERT_TRUE(dm.is_allocated(page_id));
        ASSERT_NE(page, nullptr);
        
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
        bpmt.write_page(page, COM_PG_HEADER_SZ, s1, size1);

        // flush
        ASSERT_TRUE(bpmt.flush_page_test(page->get_page_id()));

        // read with DiskManager
        char page_buf[PAGE_SIZE];
        ASSERT_TRUE(dm.read_page(page->get_page_id(), page_buf));

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
        bpmt.unpin_page_test(page_id);
        ASSERT_TRUE(bpmt.delete_page_test(page_id));
        EXPECT_FALSE(bpmt.is_in_bpm(page_id));
        EXPECT_TRUE(dm.is_free(page_id));
        EXPECT_FALSE(dm.is_allocated(page_id));
    }
}

} // namespace dawn
