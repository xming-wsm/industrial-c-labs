/**
 * @file hash_table.h
 * @brief Lab 4 - 哈希表（参数名 → 寄存器地址映射）API 声明。
 *
 * 工业背景
 * --------
 * 注塑机有成百上千个"参数"：保压压力、射胶速度、模具温度、计量位置……
 * HMI / 上位机用**参数名**（字符串）来读写，而底层寄存器 / Modbus 用
 * **地址**（数字）。两者之间需要一张快速查找表：
 *     "inj_pressure" -> 0x0010
 *     "mold_temp"    -> 0x0021
 * 用线性数组查找是 O(n)，参数一多就慢。哈希表能做到接近 O(1) 的查找。
 *
 * 设计要点
 * --------
 *   - 链地址法（separate chaining）：每个桶是一条链表，冲突的键挂在同一桶。
 *   - 节点来自**静态内存池**（复用 Lab3 的思路，不 malloc）。
 *   - 字符串键定长拷贝进节点（HT_KEY_MAX 上限），避免悬空指针。
 *
 * 设计约束
 * --------
 *   1. 不使用 malloc/free：桶数组与节点池都由调用者提供。
 *   2. 函数对 NULL 参数安全。
 */
#ifndef LAB04_HASH_TABLE_H
#define LAB04_HASH_TABLE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 键的最大长度（含结尾 '\0'）。超长的键会被截断。 */
#define HT_KEY_MAX 24

/** 操作返回状态码。 */
typedef enum {
    HT_OK = 0,          /**< 成功                         */
    HT_ERR_NULL,        /**< 传入了 NULL 指针             */
    HT_ERR_FULL,        /**< 节点池耗尽，无法插入新键     */
    HT_ERR_NOT_FOUND    /**< 未找到指定键                 */
} ht_status_t;

/** 哈希表节点：一个 键->值 条目，next 串起同桶的冲突链。 */
typedef struct ht_entry {
    char             key[HT_KEY_MAX];   /**< 参数名（定长拷贝） */
    uint16_t         value;             /**< 寄存器地址         */
    struct ht_entry *next;              /**< 同桶下一条目       */
} ht_entry_t;

/**
 * 哈希表控制块。
 * 字段公开仅为方便测试与教学，正常使用请只通过 API 访问。
 */
typedef struct {
    ht_entry_t **buckets;    /**< 桶数组：每个元素是一条冲突链的头 */
    size_t       nbuckets;   /**< 桶个数                           */
    ht_entry_t  *pool;       /**< 节点池（调用者提供）             */
    size_t       capacity;   /**< 节点池容量（最多条目数）         */
    ht_entry_t  *free_head;  /**< 空闲节点链表头                   */
    size_t       count;      /**< 当前条目数                       */
} hash_table_t;

/**
 * djb2 字符串哈希。
 * @return key 的 32 位哈希；key 为 NULL 时返回 0。
 *         （桶下标 = ht_hash(key) % nbuckets）
 */
uint32_t ht_hash(const char *key);

/**
 * 初始化哈希表。
 *
 * @param ht        控制块。
 * @param buckets   桶指针数组（至少 nbuckets 个，会被清成 NULL）。
 * @param nbuckets  桶个数，必须 > 0。
 * @param pool      节点池数组（至少 capacity 个）。
 * @param capacity  节点池容量，必须 > 0。
 * @return HT_OK；任一指针为 NULL 或 nbuckets/capacity 为 0 -> HT_ERR_NULL。
 */
ht_status_t ht_init(hash_table_t *ht,
                    ht_entry_t **buckets, size_t nbuckets,
                    ht_entry_t *pool, size_t capacity);

/** 清空所有条目（桶清 NULL，节点全部还回池）。ht 为 NULL 时安全返回。 */
void ht_reset(hash_table_t *ht);

/** @return 当前条目数；ht 为 NULL 时返回 0。 */
size_t ht_count(const hash_table_t *ht);

/** @return 节点池容量；ht 为 NULL 时返回 0。 */
size_t ht_capacity(const hash_table_t *ht);

/**
 * 插入或更新：键已存在则更新其 value；不存在则新增。
 * @return HT_OK；ht/key 为 NULL -> HT_ERR_NULL；
 *         新增但池耗尽 -> HT_ERR_FULL。
 */
ht_status_t ht_put(hash_table_t *ht, const char *key, uint16_t value);

/**
 * 查找键对应的值。
 * @param out_value 命中时写入 *out_value（可为 NULL）。
 * @return HT_OK 找到；HT_ERR_NULL（ht/key 为 NULL）；HT_ERR_NOT_FOUND。
 */
ht_status_t ht_get(const hash_table_t *ht, const char *key, uint16_t *out_value);

/**
 * 删除键。
 * @return HT_OK；HT_ERR_NULL（ht/key 为 NULL）；HT_ERR_NOT_FOUND。
 */
ht_status_t ht_remove(hash_table_t *ht, const char *key);

#ifdef __cplusplus
}
#endif

#endif /* LAB04_HASH_TABLE_H */
