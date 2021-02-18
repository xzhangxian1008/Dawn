#include "manager/db_manager.h"

namespace dawn {

DBManager *db_manager = nullptr;
size_t_ DBManager::DEFAULT_POOL_SIZE = 1000000; // 1000000 pages, approximate 40MB

} // namespace dawn