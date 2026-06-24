/**
 * @file test_hash_table.c
 * @brief Lab 4 测试套件。无需修改本文件；实现 hash_table.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "hash_table.h"

/* 注意：桶数特意取得小，以制造哈希冲突，考验冲突链处理。 */

static int test_init_basic(void) {
    ht_entry_t *buckets[4];
    ht_entry_t pool[8];
    hash_table_t ht;
    ASSERT_EQ_INT(HT_OK, ht_init(&ht, buckets, 4, pool, 8));
    ASSERT_EQ_SIZE(0u, ht_count(&ht));
    ASSERT_EQ_SIZE(8u, ht_capacity(&ht));
    return 0;
}

static int test_init_null(void) {
    ht_entry_t *buckets[4];
    ht_entry_t pool[8];
    hash_table_t ht;
    ASSERT_EQ_INT(HT_ERR_NULL, ht_init(NULL, buckets, 4, pool, 8));
    ASSERT_EQ_INT(HT_ERR_NULL, ht_init(&ht, NULL, 4, pool, 8));
    ASSERT_EQ_INT(HT_ERR_NULL, ht_init(&ht, buckets, 0, pool, 8));
    ASSERT_EQ_INT(HT_ERR_NULL, ht_init(&ht, buckets, 4, NULL, 8));
    ASSERT_EQ_INT(HT_ERR_NULL, ht_init(&ht, buckets, 4, pool, 0));
    return 0;
}

static int test_hash_deterministic(void) {
    /* 同一字符串两次哈希必须一致；NULL 返回 0。 */
    ASSERT_TRUE(ht_hash("inj_pressure") == ht_hash("inj_pressure"));
    ASSERT_EQ_INT(0, (int)ht_hash(NULL));
    /* 不同字符串通常不同（djb2 对这两个不会撞）。 */
    ASSERT_TRUE(ht_hash("mold_temp") != ht_hash("inj_pressure"));
    return 0;
}

static int test_null_safe(void) {
    uint16_t v = 0;
    ASSERT_EQ_SIZE(0u, ht_count(NULL));
    ASSERT_EQ_SIZE(0u, ht_capacity(NULL));
    ASSERT_EQ_SIZE(0u, ht_available(NULL));
    ASSERT_EQ_INT(HT_ERR_NULL, ht_put(NULL, "a", 1));
    ASSERT_EQ_INT(HT_ERR_NULL, ht_get(NULL, "a", &v));
    ASSERT_EQ_INT(HT_ERR_NULL, ht_remove(NULL, "a"));
    ASSERT_EQ_INT(7, (int)ht_get_or_default(NULL, "a", 7)); /* NULL 返回默认 */
    ht_for_each(NULL, NULL, NULL);                          /* 不应崩溃 */
    ht_reset(NULL);
    return 0;
}

static int test_put_get(void) {
    ht_entry_t *buckets[8];
    ht_entry_t pool[8];
    hash_table_t ht;
    ht_init(&ht, buckets, 8, pool, 8);

    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "inj_pressure", 0x0010));
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "mold_temp", 0x0021));
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "screw_pos", 0x0030));
    ASSERT_EQ_SIZE(3u, ht_count(&ht));

    uint16_t v = 0;
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "mold_temp", &v));
    ASSERT_EQ_INT(0x0021, v);
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "inj_pressure", &v));
    ASSERT_EQ_INT(0x0010, v);
    ASSERT_EQ_INT(HT_ERR_NOT_FOUND, ht_get(&ht, "nonexistent", &v));
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "screw_pos", NULL)); /* out 可为 NULL */
    return 0;
}

static int test_update_existing(void) {
    ht_entry_t *buckets[8];
    ht_entry_t pool[8];
    hash_table_t ht;
    ht_init(&ht, buckets, 8, pool, 8);

    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "k", 1));
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "k", 999)); /* 同 key 更新 */
    ASSERT_EQ_SIZE(1u, ht_count(&ht));           /* 数量不变 */
    uint16_t v = 0;
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "k", &v));
    ASSERT_EQ_INT(999, v);
    return 0;
}

/* ---- available：随插入/删除变化 ---- */

static int test_available(void) {
    ht_entry_t *buckets[8];
    ht_entry_t pool[4];
    hash_table_t ht;
    ht_init(&ht, buckets, 8, pool, 4);
    ASSERT_EQ_SIZE(4u, ht_available(&ht));
    ht_put(&ht, "a", 1);
    ht_put(&ht, "b", 2);
    ASSERT_EQ_SIZE(2u, ht_available(&ht));
    ht_put(&ht, "a", 11);                /* 更新不消耗槽位 */
    ASSERT_EQ_SIZE(2u, ht_available(&ht));
    ht_remove(&ht, "a");
    ASSERT_EQ_SIZE(3u, ht_available(&ht));
    return 0;
}

/* ---- get_or_default ---- */

static int test_get_or_default(void) {
    ht_entry_t *buckets[8];
    ht_entry_t pool[8];
    hash_table_t ht;
    ht_init(&ht, buckets, 8, pool, 8);
    ht_put(&ht, "exists", 42);

    ASSERT_EQ_INT(42, (int)ht_get_or_default(&ht, "exists", 999));
    ASSERT_EQ_INT(999, (int)ht_get_or_default(&ht, "missing", 999));
    return 0;
}

/* ---- for_each：遍历到所有条目（顺序不保证） ---- */

struct ht_collect {
    size_t   n;
    uint32_t value_sum;
};

static void ht_collect_cb(const char *key, uint16_t value, void *ctx) {
    struct ht_collect *c = (struct ht_collect *)ctx;
    (void)key;
    c->n++;
    c->value_sum += value;
}

static int test_for_each(void) {
    ht_entry_t *buckets[2];   /* 小桶制造冲突，确保跨链遍历 */
    ht_entry_t pool[8];
    hash_table_t ht;
    ht_init(&ht, buckets, 2, pool, 8);
    ht_put(&ht, "a", 10);
    ht_put(&ht, "bb", 20);
    ht_put(&ht, "ccc", 30);
    ht_put(&ht, "dddd", 40);

    struct ht_collect c = {0, 0};
    ht_for_each(&ht, ht_collect_cb, &c);
    ASSERT_EQ_SIZE(4u, c.n);             /* 每条恰好访问一次 */
    ASSERT_EQ_INT(100, (int)c.value_sum); /* 10+20+30+40 */
    return 0;
}

/* ---- 超长键：截断到 HT_KEY_MAX-1，仍可用同串读回 ---- */

static int test_long_key(void) {
    ht_entry_t *buckets[8];
    ht_entry_t pool[8];
    hash_table_t ht;
    ht_init(&ht, buckets, 8, pool, 8);

    /* 远超 HT_KEY_MAX 的键 */
    const char *longk = "this_is_a_very_long_parameter_name_exceeding_limit";
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, longk, 0x1234));
    uint16_t v = 0;
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, longk, &v));
    ASSERT_EQ_INT(0x1234, v);
    return 0;
}

/* ---- 冲突链：桶很少时多个键落到同一桶 ---- */

static int test_collisions(void) {
    ht_entry_t *buckets[1];   /* 只有 1 个桶：所有键都冲突，全挂一条链 */
    ht_entry_t pool[6];
    hash_table_t ht;
    ht_init(&ht, buckets, 1, pool, 6);

    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "a", 1));
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "bb", 2));
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "ccc", 3));
    ASSERT_EQ_SIZE(3u, ht_count(&ht));

    uint16_t v = 0;
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "a", &v));   ASSERT_EQ_INT(1, v);
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "bb", &v));  ASSERT_EQ_INT(2, v);
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "ccc", &v)); ASSERT_EQ_INT(3, v);
    return 0;
}

static int test_remove(void) {
    ht_entry_t *buckets[1];   /* 同桶，考验从冲突链中间删除 */
    ht_entry_t pool[6];
    hash_table_t ht;
    ht_init(&ht, buckets, 1, pool, 6);
    ht_put(&ht, "a", 1);
    ht_put(&ht, "bb", 2);
    ht_put(&ht, "ccc", 3);

    ASSERT_EQ_INT(HT_OK, ht_remove(&ht, "bb"));   /* 删中间 */
    ASSERT_EQ_SIZE(2u, ht_count(&ht));
    ASSERT_EQ_INT(HT_ERR_NOT_FOUND, ht_get(&ht, "bb", NULL));
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "a", NULL));
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "ccc", NULL));
    ASSERT_EQ_INT(HT_ERR_NOT_FOUND, ht_remove(&ht, "zzz"));
    return 0;
}

static int test_remove_recycles(void) {
    ht_entry_t *buckets[4];
    ht_entry_t pool[2];       /* 池只有 2 个节点 */
    hash_table_t ht;
    ht_init(&ht, buckets, 4, pool, 2);

    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "x", 1));
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "y", 2));
    ASSERT_EQ_INT(HT_ERR_FULL, ht_put(&ht, "z", 3)); /* 池满 */

    ASSERT_EQ_INT(HT_OK, ht_remove(&ht, "x"));       /* 腾出 */
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "z", 30));      /* 复用节点 */
    uint16_t v = 0;
    ASSERT_EQ_INT(HT_OK, ht_get(&ht, "z", &v));
    ASSERT_EQ_INT(30, v);
    return 0;
}

static int test_reset(void) {
    ht_entry_t *buckets[4];
    ht_entry_t pool[4];
    hash_table_t ht;
    ht_init(&ht, buckets, 4, pool, 4);
    ht_put(&ht, "a", 1);
    ht_put(&ht, "b", 2);
    ht_reset(&ht);
    ASSERT_EQ_SIZE(0u, ht_count(&ht));
    ASSERT_EQ_INT(HT_ERR_NOT_FOUND, ht_get(&ht, "a", NULL));
    /* reset 后还能用 */
    ASSERT_EQ_INT(HT_OK, ht_put(&ht, "c", 3));
    ASSERT_EQ_SIZE(1u, ht_count(&ht));
    return 0;
}

/* ---- 压力：反复 put/remove，验证池不泄漏 ---- */

static int test_stress_put_remove(void) {
    ht_entry_t *buckets[8];
    ht_entry_t pool[16];
    hash_table_t ht;
    ht_init(&ht, buckets, 8, pool, 16);

    char key[8];
    for (int round = 0; round < 100; round++) {
        for (int i = 0; i < 16; i++) {
            key[0] = 'k';
            key[1] = (char)('0' + i / 10);
            key[2] = (char)('0' + i % 10);
            key[3] = '\0';
            ASSERT_EQ_INT(HT_OK, ht_put(&ht, key, (uint16_t)(round + i)));
        }
        ASSERT_EQ_SIZE(16u, ht_count(&ht));
        ASSERT_EQ_INT(HT_ERR_FULL, ht_put(&ht, "overflow", 0));

        for (int i = 0; i < 16; i++) {
            key[0] = 'k';
            key[1] = (char)('0' + i / 10);
            key[2] = (char)('0' + i % 10);
            key[3] = '\0';
            ASSERT_EQ_INT(HT_OK, ht_remove(&ht, key));
        }
        ASSERT_EQ_SIZE(0u, ht_count(&ht));
        ASSERT_EQ_SIZE(16u, ht_available(&ht));
    }
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_basic);
    RUN_TEST(test_init_null);
    RUN_TEST(test_hash_deterministic);
    RUN_TEST(test_null_safe);
    RUN_TEST(test_put_get);
    RUN_TEST(test_update_existing);
    RUN_TEST(test_available);
    RUN_TEST(test_get_or_default);
    RUN_TEST(test_collisions);
    RUN_TEST(test_for_each);
    RUN_TEST(test_long_key);
    RUN_TEST(test_remove);
    RUN_TEST(test_remove_recycles);
    RUN_TEST(test_reset);
    RUN_TEST(test_stress_put_remove);
    TEST_END();
}
