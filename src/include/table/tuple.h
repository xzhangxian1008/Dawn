#pragma once

#include <vector>
#include <assert.h>
   
#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/rid.h"
#include "data/values.h"
#include "table/schema.h"

namespace dawn {

/**
 * ATTENTION no lock!
 */
class Tuple {
friend class TablePage;
public:
    Tuple() = default;

    Tuple(const RID &rid, size_t_ size) : rid_(rid), size_(size) {
        if (size_ > 0) {
            allocated_ = true;
            data_ = new char[size_];
        }
    }

    Tuple(std::vector<Value> *values, const Schema &schema) {
        init(values, schema);
    }

    Tuple(std::vector<Value> *values, const Schema &schema, const RID &rid) : rid_(rid) {
        init(values, schema);
    }

    // deep copy
    Tuple(const Tuple &tuple) {
        if (allocated_) {
            delete[] data_;
        }
        allocated_ = tuple.allocated_;
        size_ = tuple.size_;
        rid_ = tuple.rid_;
        if (allocated_) {
            data_ = new char[size_];
            memcpy(data_, tuple.data_, size_);
        }
    }

    ~Tuple() {
        if (allocated_) {
            delete[] data_;
        }
    }

    void reconstruct(std::vector<Value> *values, const Schema &schema) {
        if (allocated_) {
            delete[] data_;
        }
        init(values, schema);
    }
    
    inline size_t_ get_size() const { return size_; }
    inline char* get_data() const { return data_; }

    inline RID get_rid() const { return rid_; }
    inline void set_rid(const RID &rid) { rid_ = rid; }

    Value get_value(const Schema &schema, int idx) const;
    void set_value(const Schema &schema, Value *value, int idx);

    inline bool is_allocated() const { return allocated_; }
    string_t to_string(const Schema &schema) const;

    inline void serialize_to(char *dst) const {
        if (allocated_) {
            memcpy(dst, data_, size_);
        }
    }

    inline void deserialize_from(char *src) {
        if (allocated_) {
            memcpy(data_, src, size_);
        } else {
            // TODO: handle allocated_ == false
            assert(0);
        }
    }

    /** only compare the values of two tuples, not including rid */
    bool operator==(const Tuple &tuple) const;

    /** compare all of the fields of two tuples, including rid */
    bool is_equal(const Tuple &tuple) const;

    // deep copy
    Tuple& operator=(const Tuple &tuple) {
        if (allocated_) {
            delete[] data_;
        }
        allocated_ = tuple.allocated_;
        size_ = tuple.size_;
        rid_ = tuple.rid_;
        if (allocated_) {
            data_ = new char[size_];
            memcpy(data_, tuple.data_, size_);
        }
        return *this;
    }

private:
    /**
     * Order of the values in values is important, and
     * each value in values should be correspond to the schema.
     */
    void init(std::vector<Value> *values, const Schema &schema);

    bool allocated_ = false;
    RID rid_;
    size_t_ size_ = -1;
    char *data_;
};

} // namespace dawn
