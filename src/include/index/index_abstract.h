#pragma once

#include <vector>

namespace dawn {

// TODO add transaction
template<typename KeyType, typename ValueType, typename KeyComparator>
class IndexAbstract {
public:
    virtual ~IndexAbstract() = default;

    /**
     * insert value, if the key has existed, update it
     */
    virtual bool insert(const KeyType &key, const ValueType &value) = 0;

    virtual bool remove(const KeyType &key, const ValueType &value) = 0;

    virtual bool get_value(const KeyType &key, std::vector<ValueType> *result) = 0;
};

} // namespace dawn
