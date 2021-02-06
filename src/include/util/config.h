#pragma once

#include <string>
#include <fstream>

namespace dawn {

#define READ_DB_BUF_SZ  409600 // 100 pages
#define READ_DB_PG_NUM     100

// page
#define PAGE_SIZE            4096
#define COM_PG_HEADER_SZ       64 // page's comman header size
#define INVALID_PAGE_ID        -1
#define INVALID_SLOT_NUM       -1
#define STATUS_EXIST            1
#define STATUS_FREE             2
#define STATUS_OFFSET           0
#define LSN_OFFSET              1
#define PAGE_ID_OFFSET          5
#define TABLE_PAGE_RESERVED    64

// replacer
#define FRAME_NOT_EXIST    1
#define FRAME_EXIST_TRUE   2
#define FRAME_EXIST_FALSE  4

// index
#define LINK_HASH 1
#define BP_TREE   2

// type
#define TYPE_NUM  4

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

using page_id_t = int32_t;
using offset_t = int32_t;
using lsn_t = int32_t;
using string_t = std::string;
using fstream_t = std::fstream;
using frame_id_t = int32_t;
using hash_t = uint64_t;
using table_id_t = page_id_t;
using size_t_ = int32_t; // I think it's a stupid name

using boolean_t = bool;
using integer_t = int32_t;
using decimal_t = double;
using varchar_t = char*;

using enum_size_t = int32_t;

static constexpr size_t_ PTR_SIZE = sizeof(char*);
static constexpr size_t_ ENUM_SIZE = sizeof(enum_size_t);
static constexpr size_t_ SIZE_T_SIZE = sizeof(size_t_); // may be this is a stupid name
static constexpr size_t_ OFFSET_T_SIZE = sizeof(offset_t);
static constexpr size_t_ PGID_T_SIZE = sizeof(page_id_t);
static constexpr size_t_ DECIMAL_T_SIZE = sizeof(decimal_t);
static constexpr size_t_ INTEGER_T_SIZE = sizeof(integer_t);
static constexpr size_t_ BOOLEAN_T_SIZE = sizeof(boolean_t);

/** number of slots a LinkHashPage could contain */
static offset_t constexpr LK_HA_TOTAL_SLOT_NUM = (PAGE_SIZE - COM_PG_HEADER_SZ) / PGID_T_SIZE;

} // namespace dawn