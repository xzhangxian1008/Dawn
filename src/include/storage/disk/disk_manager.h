#include <fstream>
#include <string>

#include "util/util.h"
#include "util/config.h"

namespace dawn {
class DiskManager {
public:
    /**
     * @param file_name the file name of the database file to write to
     * @param way open or create the file? [true: create] [false: open]
     */
    explicit DiskManager(std::string &file_name, bool create);

    void write_page(page_id_t page_id, const char *src);

    void read_page(page_id_t page_id, const char *dst);

    // bool write_log();

    // bool read_log();
    
private:
    std::fstream log_io_;
    std::string log_name_;
    std::fstream file_io_;
    std::string file_name_;
};
} // namespace dawn