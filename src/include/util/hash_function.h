#pragma once

#include "util/config.h"
#include "murmur3/MurmurHash3.h"

namespace dawn {

template<typename KeyType>
class HashFunction {
public:
    hash_t do_hash(KeyType key, size_t_ key_size) {
        hash_t hash[2];
        murmur3::MurmurHash3_x64_128(reinterpret_cast<const void *>(&key), key_size, 0,
                                    reinterpret_cast<void *>(&hash));
        return hash[0];
    }
};

} // namespace dawn
