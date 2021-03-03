#pragma once

#include <string>
#include <fstream>

namespace dawn {

constexpr int32_t PAGE_SIZE =   4096;
constexpr long READ_DB_PG_NUM = 10240;
constexpr long READ_DB_BUF_SZ = PAGE_SIZE * READ_DB_PG_NUM; // 10240 pages, approximate 40MB

// page
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
#define LINK_HASH 1 // link hash
#define BP_TREE   2 // B+ tree

// type
#define TYPE_NUM  4

// op_code
#define OP_SUCCESS            0
#define DUP_KEY              -1 // key is duplicated
#define NEW_PG_FAIL          -2 // get new page fail
#define TUPLE_NOT_FOUND      -3 // can't find tuple
#define MARK_DELETE_FAIL     -4

#define INVALID_T TypeId::INVALID
#define BOOLEAN_T TypeId::BOOLEAN
#define INTEGER_T TypeId::INTEGER
#define DECIMAL_T TypeId::DECIMAL
#define VARCHAR_T TypeId::VARCHAR

#define CMP_EQ(type, left, right) Type::get_instance(type)->cmp_eq(left, right)
#define CMP_NOT_EQ(type, left, right) Type::get_instance(type)->cmp_not_eq(left, right)
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

#define INSERT_TUPLE_FUNC_PARAMS     page_id_t first_page_id, Tuple *tuple, const Schema &tb_schema, BufferPoolManager *bpm
#define MARK_DELETE_FUNC_PARAMS      page_id_t first_page_id, Value key_value, const Schema &tb_schema, BufferPoolManager *bpm
#define APPLY_DELETE_FUNC_PARAMS     page_id_t first_page_id, Value key_value, const Schema &tb_schema, BufferPoolManager *bpm
#define ROLLBACK_DELETE_FUNC_PARAMS  page_id_t first_page_id, Value key_value, const Schema &tb_schema, BufferPoolManager *bpm
#define GET_TUPLE_FUNC_PARAMS        page_id_t first_page_id, Value key_value, Tuple *tuple, const Schema &tb_schema, BufferPoolManager *bpm
#define UPDATE_TUPLE_FUNC_PARAMS     page_id_t first_page_id, Tuple *new_tuple, const RID &old_rid, const Schema &tb_schema, BufferPoolManager *bpm

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
using op_code_t = int32_t; // operation result
using index_code_t = int32_t; // show the type of index

static constexpr size_t_ PTR_SIZE = sizeof(char*);
static constexpr size_t_ ENUM_SIZE = sizeof(enum_size_t);
static constexpr size_t_ SIZE_T_SIZE = sizeof(size_t_); // may be this is a stupid name
static constexpr size_t_ OFFSET_T_SIZE = sizeof(offset_t);
static constexpr size_t_ PGID_T_SIZE = sizeof(page_id_t);
static constexpr size_t_ DECIMAL_T_SIZE = sizeof(decimal_t);
static constexpr size_t_ INTEGER_T_SIZE = sizeof(integer_t);
static constexpr size_t_ BOOLEAN_T_SIZE = sizeof(boolean_t);

/** number of slots a LinkHashPage could contain */
static offset_t constexpr LK_HA_PG_SLOT_NUM = (PAGE_SIZE - COM_PG_HEADER_SZ) / PGID_T_SIZE;

/** total slot number the link hash function has */
static offset_t constexpr Lk_HA_TOTAL_SLOT_NUM = LK_HA_PG_SLOT_NUM * LK_HA_PG_SLOT_NUM;

} // namespace dawn