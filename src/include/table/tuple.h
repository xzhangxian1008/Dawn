#include <vector>

#include "util/util.h"
#include "util/config.h"
#include "util/rwlatch.h"
#include "table/rid.h"
#include "data/values.h"
#include "table/table_schema.h"

namespace dawn {

/**
 * ATTENTION no lock!
 */
class Tuple {
public:
    Tuple() = default;
    Tuple(std::vector<Value> &values, const TableSchema &schema);
    Tuple(std::vector<Value> &values, const TableSchema &schema, const RID &rid) : rid_(rid) { Tuple(values, schema); }

    ~Tuple() {
        if (allocated_) {
            delete data_;
        }
    }
    
    inline size_t_ get_size() const { return size_; }
    inline char* get_data() const { return data_; }

    inline RID get_rid() const { return rid_; }
    inline void set_rid(const RID &rid) { rid_ = rid; }

    Value get_value(const TableSchema &schema, int idx);
    void set_value(const TableSchema &schema, Value &value, int idx);

    string_t to_string(const TableSchema *schema);

private:
    bool allocated_ = false;
    RID rid_;
    size_t_ size_ = -1;
    char *data_;
};

} // namespace dawn
