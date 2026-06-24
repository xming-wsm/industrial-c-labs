/**
 * @file hash_table.c
 * @brief Lab 4 - 哈希表（待你完成）
 *
 * 详见 docs/lab04_hash_table.md；测试：xmake lab4 test
 */
#include "hash_table.h"
#include <string.h>

uint32_t ht_hash(const char *key) {
    (void)key;
    return 0;
}

ht_status_t ht_init(hash_table_t *ht,
                    ht_entry_t **buckets, size_t nbuckets,
                    ht_entry_t *pool, size_t capacity) {
    (void)ht;
    (void)buckets;
    (void)nbuckets;
    (void)pool;
    (void)capacity;
    return HT_ERR_NULL;
}

void ht_reset(hash_table_t *ht) {
    (void)ht;
}

size_t ht_count(const hash_table_t *ht) {
    (void)ht;
    return 0;
}

size_t ht_capacity(const hash_table_t *ht) {
    (void)ht;
    return 0;
}

size_t ht_available(const hash_table_t *ht) {
    (void)ht;
    return 0;
}

ht_status_t ht_put(hash_table_t *ht, const char *key, uint16_t value) {
    (void)ht;
    (void)key;
    (void)value;
    return HT_ERR_NULL;
}

ht_status_t ht_get(const hash_table_t *ht, const char *key, uint16_t *out_value) {
    (void)ht;
    (void)key;
    (void)out_value;
    return HT_ERR_NULL;
}

uint16_t ht_get_or_default(const hash_table_t *ht, const char *key, uint16_t def) {
    (void)ht;
    (void)key;
    return def;
}

ht_status_t ht_remove(hash_table_t *ht, const char *key) {
    (void)ht;
    (void)key;
    return HT_ERR_NULL;
}

void ht_for_each(const hash_table_t *ht, ht_visit_fn fn, void *ctx) {
    (void)ht;
    (void)fn;
    (void)ctx;
}
