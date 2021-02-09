#include "manager/db_manager.h"

namespace dawn {

DBManager *db_manager = nullptr;
size_t_ DBManager::DEFAULT_POOL_SIZE = 50;

} // namespace dawn