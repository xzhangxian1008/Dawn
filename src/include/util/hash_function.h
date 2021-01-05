#pragma once

#include "util/config.h"

namespace dawn {

template<typename KeyType>
class HashFunction {
public:
    hash_slot_t do_hash(KeyType key) {
        hash_slot_t hash[2];
        murmur3::MurmurHash3_x64_128(reinterpret_cast<const void *>(&key), static_cast<int>(sizeof(KeyType)), 0,
                                    reinterpret_cast<void *>(&hash));
        return hash[0];
    }
};
    
} // namespace dawn
