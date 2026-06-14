/**
 * @file hash_table.c
 * @brief Lab 4 - 哈希表实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_hash_table.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab04_hash_table.md）：
 *   - ht_hash：djb2，hash = 5381; 对每个字符 hash = hash * 33 + c。
 *   - 桶下标：ht_hash(key) % nbuckets。
 *   - 节点池与 Lab3 一样用空闲链表：init 时串好，put 取、remove 还。
 *   - 键用 strncmp 比较、用定长拷贝写入（注意结尾 '\0'）。
 *   - 全程不使用 malloc/free。
 */
#include "hash_table.h"
#include <string.h>

uint32_t ht_hash(const char *key) {
    /* TODO:
     *   key 为 NULL -> 返回 0。
     *   djb2：uint32_t h = 5381; 对每个字符 c： h = h * 33 + (unsigned char)c;
     *   返回 h。
     */
    (void)key;
    return 0;
}

ht_status_t ht_init(hash_table_t *ht,
                    ht_entry_t **buckets, size_t nbuckets,
                    ht_entry_t *pool, size_t capacity) {
    /* TODO:
     *   1. 任一指针为 NULL，或 nbuckets==0 或 capacity==0 -> HT_ERR_NULL
     *   2. 绑定字段；把 buckets[0..nbuckets-1] 全清成 NULL；
     *      把 pool[0..capacity-1] 串成空闲链表，free_head 指向首个；
     *      count = 0 -> HT_OK
     */
    (void)ht;
    (void)buckets;
    (void)nbuckets;
    (void)pool;
    (void)capacity;
    return HT_ERR_NULL; /* 占位：实现前测试应当失败 */
}

void ht_reset(hash_table_t *ht) {
    /* TODO: 桶全清 NULL；节点全部重新串成空闲链表；count = 0。 */
    (void)ht;
}

size_t ht_count(const hash_table_t *ht) {
    /* TODO: NULL 返回 0；否则返回 count。 */
    (void)ht;
    return 0;
}

size_t ht_capacity(const hash_table_t *ht) {
    /* TODO: NULL 返回 0；否则返回 capacity。 */
    (void)ht;
    return 0;
}

ht_status_t ht_put(hash_table_t *ht, const char *key, uint16_t value) {
    /* TODO:
     *   ht/key 为 NULL -> HT_ERR_NULL。
     *   算桶下标；在该桶链上找 key：
     *     - 找到：更新 value，返回 HT_OK（count 不变）。
     *     - 没找到：从空闲链表取一个节点（取不到 -> HT_ERR_FULL）；
     *       拷贝 key（注意 '\0'），写 value，插到桶链头部；count++，返回 HT_OK。
     */
    (void)ht;
    (void)key;
    (void)value;
    return HT_ERR_NULL;
}

ht_status_t ht_get(const hash_table_t *ht, const char *key, uint16_t *out_value) {
    /* TODO:
     *   ht/key 为 NULL -> HT_ERR_NULL。
     *   算桶下标，遍历桶链找 key：
     *     找到 -> out_value 非 NULL 时写入 value，返回 HT_OK；
     *     没找到 -> HT_ERR_NOT_FOUND。
     */
    (void)ht;
    (void)key;
    (void)out_value;
    return HT_ERR_NULL;
}

ht_status_t ht_remove(hash_table_t *ht, const char *key) {
    /* TODO:
     *   ht/key 为 NULL -> HT_ERR_NULL。
     *   算桶下标，在桶链上找 key（维护前驱指针）：
     *     找到 -> 从桶链摘除、节点还回空闲链表、count--，返回 HT_OK；
     *     没找到 -> HT_ERR_NOT_FOUND。
     */
    (void)ht;
    (void)key;
    return HT_ERR_NULL;
}
