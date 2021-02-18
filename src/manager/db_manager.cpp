#include "manager/db_manager.h"

namespace dawn {

DBManager *db_manager = nullptr;
size_t_ DBManager::DEFAULT_POOL_SIZE = 10240; // 10240 pages, approximate 40MB

} // namespace dawn