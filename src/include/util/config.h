#pragma once

#include <string>
#include <fstream>

namespace dawn {

#define READ_DB_BUF_SZ 409600 // 100 pages
#define READ_DB_PG_NUM 100

// page
#define PAGE_SIZE 4096
#define COM_PG_HEADER_SZ 64 // page's comman header size
#define INVALID_PAGE_ID -1
#define STATUS_EXIST 1
#define STATUS_FREE 2
#define STATUS_OFFSET 0
#define LSN_OFFSET 1
#define PAGE_ID_OFFSET 5

// replacer
#define FRAME_NOT_EXIST 1
#define FRAME_EXIST_TRUE 2
#define FRAME_EXIST_FALSE 4

using page_id_t = int32_t;
using offset_t = int32_t;
using lsn_t = int32_t;
using string_t = std::string;
using fstream_t = std::fstream;
using frame_id_t = int32_t;
using hash_slot_t = uint64_t;

} // namespace dawn