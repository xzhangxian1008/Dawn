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

// type
#define TYPE_NUM 4

#define INVALID_T TypeId::INVALID
#define BOOLEAN_T TypeId::BOOLEAN
#define INTEGER_T TypeId::INTEGER
#define DECIMAL_T TypeId::DECIMAL
#define VARCHAR_T TypeId::VARCHAR

#define CMP_EQ(type, left, right) Type::get_instance(type)->cmp_eq(left, right)
#define CMP_LESS(type, left, right) Type::get_instance(type)->cmp_less(left, right)
#define CMP_LESS_EQ(type, left, right) Type::get_instance(type)->cmp_less_and_eq(left, right)
#define CMP_GREATER(type, left, right) Type::get_instance(type)->cmp_greater(left, right)
#define CMP_GREATER_EQ(type, left, right) Type::get_instance(type)->cmp_greater_and_eq(left, right)

#define MINUS(type, left, right) Type::get_instance(type)->minus(left, right)
#define ADD(type, left, right) Type::get_instance(type)->add(left, right)
#define MULTIPLY(type, left, right) Type::get_instance(type)->multiply(left, right)
#define DIVIDE(type, left, right) Type::get_instance(type)->divide(left, right)
#define MIN(type, left, right) Type::get_instance(type)->min(left, right)
#define MAX(type, left, right) Type::get_instance(type)->max(left, right)

#define PTR_SIZE sizeof(char*)

using page_id_t = int32_t;
using offset_t = int32_t;
using lsn_t = int32_t;
using string_t = std::string;
using fstream_t = std::fstream;
using frame_id_t = int32_t;
using hash_slot_t = uint64_t;
using table_id_t = page_id_t;

using boolean_t = bool;
using integet_t = int32_t;
using decimal_t = double;
using varchar_t = char*;
} // namespace dawn