#pragma once

#include <vector>
#include "table/rid.h"
#include "table/tuple.h"

namespace dawn {

/**
 * TODO add transaction
 * not support joint index
 */
template<typename KeyType, typename KeyComparator>
class IndexAbstract {
public:
    virtual ~IndexAbstract() = default;

    /**
     * insert value, if the key has existed, update it
     */
    virtual bool insert(const KeyType &key, const RID &value) = 0;

    virtual bool remove(const KeyType &key, const RID &value) = 0;

    virtual bool get_tuple(const KeyType &key, std::vector<Tuple> *result) = 0;
};

} // namespace dawn
