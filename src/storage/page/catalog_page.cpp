#include "storage/page/catalog_page.h"

namespace dawn {

void CatalogPage::init_catalog(page_id_t page_id, BufferPoolManager *bpm) {
    page_id_t *pgid = const_cast<page_id_t*>(&self_page_id_);
    *pgid = page_id;
    bpm_ = bpm;
    page_ = bpm_->get_page(self_page_id_);
    if (page_ == nullptr) {
        PRINT("ERROR: get nullptr. page id ", self_page_id_);
        exit(-1);
    }
    data_ = page_->get_data();

    // read data for initialization
    free_space_pointer_ = PAGE_SIZE;
    int num = *reinterpret_cast<int*>(page_->get_data() + TABLE_NUM_OFFSET);
    table_num_ = num;
    int tb_name_len = 0;

    offset_t tb_record_offset;
    offset_t tb_name_offset;
    int tb_name_sz;
    int tb_page_id;
    string_t tb_name;

    // get tables' info
    for (int i = 0; i < table_num_; i++) {
        tb_record_offset = TABLE_NUM_OFFSET + 4 + i * TABLE_RECORD_SZ;
        tb_name_offset = *reinterpret_cast<offset_t*>(data_ + tb_record_offset);
        tb_name_sz = *reinterpret_cast<int*>(data_ + tb_record_offset + 4);
        tb_page_id = *reinterpret_cast<page_id_t*>(data_ + tb_record_offset + 8);
        tb_name = get_table_name(tb_name_offset, tb_name_sz);
        tb_id_to_name_.insert(std::make_pair(tb_page_id, tb_name));
        tb_name_to_id_.insert(std::make_pair(tb_name, tb_page_id));
    }

    // initialize the free_space_pointer_
    if (table_num_ != 0) {
        free_space_pointer_ = 
            *reinterpret_cast<offset_t*>(data_ + TABLE_NUM_OFFSET + 4 + (table_num_-1) * TABLE_RECORD_SZ);
    }
}

string_t CatalogPage::get_table_name(offset_t tb_name_offset, int size) {
    char tb_name[size + 1];

    for (int i = 0; i < size; i++)
        tb_name[i] = *reinterpret_cast<char*>(data_ + tb_name_offset + i);
    tb_name[size] = '\0';

    return string_t(tb_name);
}

void CatalogPage::new_table(const string_t &table_name) {

}

bool CatalogPage::delete_table(const string_t &table_name) {

}

bool CatalogPage::delete_table(table_id_t table_id) {

}

bool CatalogPage::change_table_name(table_id_t table_id, const string_t &new_name) {

}

TableMetaData* CatalogPage::get_table_meta_data(table_id_t table_id) {
    latch_.w_lock();
    auto iter = tb_id_to_meta_.find(table_id);
    if (iter == tb_id_to_meta_.end()) {
        // find his name
        auto name_iter = tb_id_to_name_.find(table_id);
        if (name_iter == tb_id_to_name_.end()) {
            // the table does not exist
            latch_.w_unlock();
            return nullptr;
        }

        // create the TableMetaData
        TableMetaData *tmd = new TableMetaData(bpm_, name_iter->second, table_id, table_id);
        tb_id_to_meta_.insert(std::make_pair(table_id, tmd));
        latch_.w_unlock();
        return tmd;
    }
    TableMetaData *tb_meta_data = iter->second;
    latch_.w_unlock();
    return tb_meta_data;
}

} // namespace dawn
