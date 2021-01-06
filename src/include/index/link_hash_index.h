#pragma once

#include "index/index_abstract.h"
#include "buffer/buffer_pool_manager.h"
#include "util/rwlatch.h"
#include "util/config.h"
#include "util/util.h"
#include "util/hash_function.h"

namespace dawn {

template<typename KeyType, typename ValueType, typename KeyComparator>
class LinkHashIndex : public IndexAbstract<KeyType, ValueType, KeyComparator> {
public:

    /**
     * insert value, if the key has existed, update it
     */
    virtual bool insert(const KeyType &key, const ValueType &value);

    virtual bool remove(const KeyType &key, const ValueType &value);

    /**
     * maybe not only one value has been hashed to that bucket
     */
    virtual bool get_value(const KeyType &key, std::vector<ValueType> *result);

private:
    page_id_t link_hash_page_id_;
    KeyComparator key_comparator_;
    BufferPoolManager *buffer_pool_manager_;
    HashFunction<KeyType> hash_function_;
    ReaderWriterLatch latch_;
};

} // namespace dawn
