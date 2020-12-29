#include <fstream>
#include <string>
#include <atomic>

#include "util/util.h"
#include "util/config.h"

namespace dawn {
class DiskManager {
public:
    /**
     * 
     * @param db_name the file name of the database file to write to
     * @param way open or create the file? [true: create] [false: open]
     */
    explicit DiskManager(std::string &db_name, bool create = true);

    bool write_page(page_id_t page_id, const char *data);
    bool read_page(page_id_t page_id, char *dst);
    page_id_t alloc_page(char *dst);
    // bool write_log();
    // bool read_log();
    bool get_status() const { return status; }
    page_id_t get_next_page_id() const { return next_page_id; }
    void shutdown();

private:
    std::fstream log_io_;
    std::string log_name_;
    std::fstream db_io_;
    std::string db_name_;
    bool status; // if the manager works normally. true: yes false: no

    // refer to the next free page id
    std::atomic<page_id_t> next_page_id; 
};
} // namespace dawn