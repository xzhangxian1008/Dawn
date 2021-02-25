#include "meta/catalog_table.h"

namespace dawn {

CatalogTable::CatalogTable(page_id_t page_id, BufferPoolManager *bpm, bool from_scratch) {
    self_page_id_ = page_id;
    bpm_ = bpm;
    page_ = bpm_->get_page(self_page_id_);
    if (page_ == nullptr) {
        PRINT("CatalogTable: ERROR! get nullptr, page id ", self_page_id_);
        exit(-1);
    }
    data_ = page_->get_data();

    if (from_scratch)
        init_catalog_table();

    // read data for initialization
    free_space_pointer_ = PAGE_SIZE;
    int num = *reinterpret_cast<int*>(data_ + TABLE_NUM_OFFSET);
    table_num_ = num;

    offset_t tb_record_offset;
    offset_t tb_name_offset;
    int tb_name_sz;
    int tb_page_id;
    string_t tb_name;

    // get tables' info
    for (int i = 0; i < table_num_; i++) {
        tb_record_offset = TABLE_NUM_OFFSET + SIZE_T_SIZE + i * TABLE_RECORD_SZ;
        tb_name_offset = *reinterpret_cast<offset_t*>(data_ + tb_record_offset);
        tb_name_sz = *reinterpret_cast<int*>(data_ + tb_record_offset + OFFSET_T_SIZE);
        tb_page_id = *reinterpret_cast<page_id_t*>(data_ + tb_record_offset + OFFSET_T_SIZE + SIZE_T_SIZE);
        tb_name = get_table_name(tb_name_offset, tb_name_sz);
        tb_id_to_name_.insert(std::make_pair(tb_page_id, tb_name));
        tb_name_to_id_.insert(std::make_pair(tb_name, tb_page_id));
    }

    // initialize the free_space_pointer_
    if (table_num_ != 0) {
        free_space_pointer_ = 
            *reinterpret_cast<offset_t*>(data_ + TABLE_NUM_OFFSET + SIZE_T_SIZE + (table_num_-1) * TABLE_RECORD_SZ);
    }
}

string_t CatalogTable::get_table_name(offset_t tb_name_offset, int size) {
    char tb_name[size + 1];

    for (int i = 0; i < size; i++)
        tb_name[i] = *reinterpret_cast<char*>(data_ + tb_name_offset + i);
    tb_name[size] = '\0';

    return string_t(tb_name);
}

bool CatalogTable::create_table(const string_t &table_name, const Schema &schema) {
    latch_.w_lock();
    // check if table_name is duplicate
    auto iter = tb_name_to_id_.find(table_name);
    if (iter != tb_name_to_id_.end()) {
        latch_.w_unlock();
        return false;
    }

    // find where to store TableMetaData's meta data and ensure there are enough space to store
    size_t_ tb_num = *reinterpret_cast<size_t_*>(data_ + TABLE_NUM_OFFSET);
    offset_t insert_offset = TABLE_NUM_OFFSET + SIZE_T_SIZE + tb_num * TABLE_RECORD_SZ;
    size_t_ available_space = free_space_pointer_ - insert_offset;
    if (static_cast<size_t_>(table_name.length() + TABLE_RECORD_SZ) > available_space) {
        latch_.w_unlock();
        return false; // no more space to store
    }

    // get a new page to for TableMetaData
    Page *new_page = bpm_->new_page();
    if (new_page == nullptr) {
        latch_.w_unlock();
        return false;
    }

    // update CatalogTable's meta data
    tb_id_to_meta_.insert(std::make_pair(new_page->get_page_id(), 
        new TableMetaData(bpm_, table_name, schema, new_page->get_page_id())));
    tb_id_to_name_.insert(std::make_pair(new_page->get_page_id(), table_name));
    tb_name_to_id_.insert(std::make_pair(table_name, new_page->get_page_id()));
    bpm_->unpin_page(new_page->get_page_id(), false);
    table_num_++;
    *reinterpret_cast<size_t_*>(data_ + TABLE_NUM_OFFSET) = table_num_;

    // persist the CatalogTable's meta data
    size_t_ len = table_name.length();
    free_space_pointer_ -= len + 1;
    for (int i = 0; i < len; i++)
        *reinterpret_cast<char*>(data_ + free_space_pointer_ + i) = table_name[i];
    *reinterpret_cast<char*>(data_ + free_space_pointer_ + len) = '\0';

    *reinterpret_cast<offset_t*>(data_ + insert_offset) = free_space_pointer_;
    *reinterpret_cast<size_t_*>(data_ + insert_offset + OFFSET_T_SIZE) = table_name.length();
    *reinterpret_cast<page_id_t*>(data_ + insert_offset + OFFSET_T_SIZE + SIZE_T_SIZE) = new_page->get_page_id();

    latch_.w_unlock();
    bpm_->flush_page(self_page_id_); // ATTENTION transaction conflict
    return true;
}

bool CatalogTable::delete_table(const string_t &table_name) {
    latch_.r_lock();
    auto iter = tb_name_to_id_.find(table_name);
    if (iter == tb_name_to_id_.end()) {
        latch_.r_unlock();
        return true;
    }
    latch_.r_unlock();
    return delete_table(iter->second);
}

bool CatalogTable::delete_table(table_id_t table_id) {
    latch_.w_lock();
    auto iter = tb_id_to_name_.find(table_id);
    if (iter == tb_id_to_name_.end()) {
        latch_.w_unlock();
        return true;
    }

    string_t deleted_tb_name = iter->second;

    // find this table's offset in the page
    offset_t tb_offset = -1;
    int i = 0; // deleted table's index
    for (; i < table_num_; i++) {
        tb_offset = TABLE_NUM_OFFSET + SIZE_T_SIZE + i * TABLE_RECORD_SZ;
        table_id_t tb_id = 
            *reinterpret_cast<table_id_t*>(data_ + tb_offset + OFFSET_T_SIZE + SIZE_T_SIZE);
        if (table_id == tb_id) {
            break;
        }
    }

    if (tb_offset == -1) {
        latch_.w_unlock();
        LOG("ERROR: Data inconsistency");
        return false;
    }

    offset_t deleted_name_size = 
        *reinterpret_cast<offset_t*>(data_ + tb_offset + OFFSET_T_SIZE);
    offset_t deleted_page_id = 
        *reinterpret_cast<offset_t*>(data_ + tb_offset + OFFSET_T_SIZE + SIZE_T_SIZE);

    // adjust the page
    if (i != table_num_ - 1) {
        offset_t name_offset;
        offset_t table_offset;
        size_t_ name_size;
        for (int j = i + 1; j < table_num_; j++) {
            table_offset = TABLE_NUM_OFFSET + SIZE_T_SIZE + j * TABLE_RECORD_SZ;
            name_offset = 
                *reinterpret_cast<offset_t*>(data_ + table_offset);
            name_size = 
                *reinterpret_cast<size_t_*>(data_ + table_offset + OFFSET_T_SIZE);

            // do move operation
            memcpy(data_ + table_offset - TABLE_RECORD_SZ, data_ + table_offset, TABLE_RECORD_SZ);
            memmove(data_ + name_offset + deleted_name_size + 1, data_ + name_offset, name_size + 1);

            // update the table name's position
            *reinterpret_cast<offset_t*>(data_ + table_offset - TABLE_RECORD_SZ) += deleted_name_size + 1;
        }
    }
    table_num_--;
    *reinterpret_cast<size_t_*>(data_ + TABLE_NUM_OFFSET) = table_num_;
    free_space_pointer_ += deleted_name_size + 1;
    delete_table_data(table_id, deleted_tb_name);
    bpm_->delete_page(deleted_page_id);
    latch_.w_unlock();
    bpm_->flush_page(self_page_id_); // ATTENTION transaction conflict
    return true;
}

bool CatalogTable::change_table_name(table_id_t table_id, const string_t &new_name)
{
        latch_.w_lock();
        auto iter = tb_id_to_name_.find(table_id);
        if(iter == tb_id_to_name_.end())
        {
            latch_.w_unlock();
            PRINT("========", "there is no table named ", table_id, "========");
            return false;
        }
        string_t old_name = iter->second;
        //find this table's offset in the page
        offset_t tb_offset = -1;
        int i = 0;
        // int table_count = table_num_;
        for(; i < table_num_; i++)
        {
            tb_offset = TABLE_NUM_OFFSET + SIZE_T_SIZE + i * TABLE_RECORD_SZ;
            table_id_t tb_id = *reinterpret_cast<table_id_t*>(data_  +tb_offset + OFFSET_T_SIZE + SIZE_T_SIZE);
            if(table_id == tb_id)
            {
                break;
            }
        }
        if(tb_offset == -1)
        {
            latch_.w_unlock();
            LOG("ERROR: Data inconsistency");
            return false;
        }
        //update the name.
        int32_t difference = new_name.length() - old_name.length();
        size_t_ difference_abs = abs(int(new_name.length() - old_name.length()));
        if(difference < 0) //the new_name's length reduced.
        {
            offset_t next_name_offset;
            offset_t next_table_offset;
            size_t_ next_name_size;

            for(int j = i + 1; j < table_num_; j++)
            {
                next_table_offset = TABLE_NUM_OFFSET + SIZE_T_SIZE + j * TABLE_RECORD_SZ;
                next_name_offset = *reinterpret_cast<offset_t*>(data_ + next_table_offset);
                next_name_size = *reinterpret_cast<size_t_*>(data_ + next_table_offset + OFFSET_T_SIZE);
                memmove(data_ + next_name_offset + difference_abs, data_ + next_name_offset, next_name_size + 1);
                //update the previous table name's position.
                *reinterpret_cast<offset_t*>(data_ + next_table_offset - TABLE_RECORD_SZ) += difference_abs;
                //update the current table name's position.
                if(j == table_num_ -1)
                    *reinterpret_cast<offset_t*>(data_ + next_table_offset) += difference_abs;
            }
            //update the free_space_pointer_.
            free_space_pointer_ += difference_abs;
        }
        else if(difference > 0)
        {
            offset_t previous_name_offset;
            offset_t previous_table_offset;
            size_t_ previous_name_size;
            //make sure there is enough space to update table's name.
            if(!check_space(difference_abs))
            {
                latch_.w_unlock();
                return false;
            }
            for(int k = table_num_ -1; k >= i; k--)
            {
                previous_table_offset = TABLE_NUM_OFFSET + SIZE_T_SIZE + k * TABLE_RECORD_SZ;
                previous_name_offset = *reinterpret_cast<offset_t*>(data_ + previous_table_offset);
                previous_name_size = *reinterpret_cast<size_t_*>(data_ + previous_table_offset + OFFSET_T_SIZE);
                memmove(data_ + previous_name_offset - difference_abs, data_ + previous_name_offset, previous_name_size + 1);
                //update the current table name's position.
                *reinterpret_cast<offset_t*>(data_ + previous_table_offset) -= difference_abs;
            }
            //update the free_space_pointer_.
            free_space_pointer_ -= difference_abs;
        }
        //update new name.
        size_t_ len = new_name.length();
        offset_t new_name_offset = *reinterpret_cast<offset_t*>(data_ + tb_offset);
        for(int index = 0; index < len; index++)
        {
            *reinterpret_cast<char*>(data_ + new_name_offset + index) = new_name[index];
        }
        *reinterpret_cast<char*>(data_ + new_name_offset + len) = '\0';
        //update the new name's size;
        *reinterpret_cast<size_t_*>(data_ + tb_offset + OFFSET_T_SIZE) = new_name.length();
        
        //update the tb_id_to_meta
        auto iter_id_to_meta = tb_id_to_meta_.find(table_id);
        if (iter_id_to_meta == tb_id_to_meta_.end()) 
        {
            iter_id_to_meta->second->set_table_name(new_name);
        }
        //update the tb_id_to_name_
        auto iter_id_to_name = tb_id_to_name_.find(table_id);
        tb_id_to_name_.erase(iter_id_to_name);
        tb_id_to_name_.insert(std::make_pair(table_id, new_name));
        //update the tb_name_to_id_
        auto iter_name_to_id = tb_name_to_id_.find(old_name);
        tb_name_to_id_.erase(iter_name_to_id);
        tb_name_to_id_.insert(std::make_pair(new_name, table_id));

        //flush page to disk.
        latch_.w_unlock();
        bpm_->flush_page(self_page_id_);
        return true; 
}


TableMetaData* CatalogTable::get_table_meta_data(table_id_t table_id) {
    latch_.w_lock();
    auto iter = tb_id_to_meta_.find(table_id);

    // read from disk if the TableMetaTable is nonexistent in memory
    if (iter == tb_id_to_meta_.end()) {
        // find his name
        auto name_iter = tb_id_to_name_.find(table_id);
        if (name_iter == tb_id_to_name_.end()) {
            // the table does not exist
            latch_.w_unlock();
            return nullptr;
        }

        // create the TableMetaData
        TableMetaData *tmd = new TableMetaData(bpm_, name_iter->second, table_id);
        tb_id_to_meta_.insert(std::make_pair(table_id, tmd));
        latch_.w_unlock();
        return tmd;
    }
    TableMetaData *tb_meta_data = iter->second;
    latch_.w_unlock();
    return tb_meta_data;
}

std::vector<string_t> CatalogTable::get_all_table_name() {
    latch_.r_lock();
    std::vector<string_t> names;
    for (auto &iter : tb_name_to_id_)
        names.push_back(iter.first);
    latch_.r_unlock();
    return names;
}

std::vector<table_id_t> CatalogTable::get_all_table_id() {
    latch_.r_lock();
    std::vector<table_id_t> ids;
    for (auto &iter : tb_name_to_id_)
        ids.push_back(iter.second);
    latch_.r_unlock();
    return ids;
}

string_t CatalogTable::to_string() {
    std::ostringstream os;

    os << "CatalogTable{"
        << "table number:" << tb_id_to_name_.size() << "}";

    bool first = true;
    os << " :: [";
    for (auto iter : tb_id_to_name_) {
        if (first) {
            first = false;
        } else {
            os << ", ";
        }
        os << iter.second;
    }
    os << "]";

    return os.str();
}

} // namespace dawn
