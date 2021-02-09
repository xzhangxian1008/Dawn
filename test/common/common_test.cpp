#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>

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
const size_t_ DEFAULT_POOL = 50;
TEST(CommonTest1, CommonTEST11) {
    const page_id_t index_header_page_id_ = 10;
    page_id_t *pgid = const_cast<page_id_t*>(&index_header_page_id_);
    *pgid = 100; // right
    size_t_ *default_pool_size = const_cast<size_t_*>(&DEFAULT_POOL);
    *default_pool_size = 100; // wrong
}

} // namespace dawn