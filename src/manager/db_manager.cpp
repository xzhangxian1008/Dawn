#include "manager/db_manager.h"
#include <memory>

namespace dawn {

std::unique_ptr<DBManager> db_manager;
size_t_ DBManager::DEFAULT_POOL_SIZE = 10240; // 10240 pages, approximate 40MB

} // namespace dawn