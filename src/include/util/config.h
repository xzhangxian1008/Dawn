#pragma once

namespace dawn {
#define PAGE_SIZE 4096
#define READ_DB_BUF_SZ 409600 // 100 pages
#define READ_DB_PG_NUM 100
#define PG_COM_HEADER_SZ 64 // page's comman header size
#define INVALID_PAGE_ID -1
#define STATUS_EXIST 1

using page_id_t = __INT32_TYPE__;
using offset_t = __INT32_TYPE__;
using lsn_t = __INT32_TYPE__;
using string_t = std::string;
using fstream_t = std::fstream;

} // namespace dawn