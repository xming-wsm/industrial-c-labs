/**
 * @file test_alarm_list.c
 * @brief Lab 3 测试套件。无需修改本文件；实现 alarm_list.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "alarm_list.h"

/* ---- 初始化与参数校验 ---- */

static int test_init_basic(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    ASSERT_EQ_INT(LL_OK, al_init(&al, pool, 4));
    ASSERT_EQ_SIZE(4u, al_capacity(&al));
    ASSERT_EQ_SIZE(0u, al_count(&al));
    ASSERT_FALSE(al_is_full(&al));
    return 0;
}

static int test_init_null(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    ASSERT_EQ_INT(LL_ERR_NULL, al_init(NULL, pool, 4));
    ASSERT_EQ_INT(LL_ERR_NULL, al_init(&al, NULL, 4));
    ASSERT_EQ_INT(LL_ERR_NULL, al_init(&al, pool, 0));
    return 0;
}

static int test_null_safe(void) {
    ASSERT_EQ_SIZE(0u, al_count(NULL));
    ASSERT_EQ_SIZE(0u, al_capacity(NULL));
    ASSERT_EQ_SIZE(0u, al_available(NULL));
    ASSERT_FALSE(al_is_full(NULL));
    ASSERT_TRUE(al_node_alloc(NULL) == NULL);
    al_node_free(NULL, NULL);
    ASSERT_EQ_INT(LL_ERR_NULL, al_add(NULL, 1, 2));
    ASSERT_FALSE(al_find(NULL, 1, NULL));
    ASSERT_EQ_INT(LL_ERR_NULL, al_update(NULL, 1, 2));
    ASSERT_EQ_INT(LL_ERR_NULL, al_remove(NULL, 1));
    ASSERT_EQ_SIZE(0u, al_remove_all(NULL, 1));
    al_for_each(NULL, NULL, NULL);   /* 不应崩溃 */
    al_reset(NULL);
    return 0;
}

/* ---- 内存池底层：alloc / free / 复用 ---- */

static int test_pool_alloc_free(void) {
    ll_node_t pool[3];
    alarm_list_t al;
    al_init(&al, pool, 3);

    ll_node_t *a = al_node_alloc(&al);
    ll_node_t *b = al_node_alloc(&al);
    ll_node_t *c = al_node_alloc(&al);
    ASSERT_TRUE(a != NULL && b != NULL && c != NULL);
    ASSERT_TRUE(a != b && b != c && a != c);   /* 分配出的是不同节点 */
    ASSERT_EQ_SIZE(3u, al_count(&al));
    ASSERT_TRUE(al_is_full(&al));
    /* 池耗尽，再分配返回 NULL */
    ASSERT_TRUE(al_node_alloc(&al) == NULL);

    /* 释放一个后又能分配 */
    al_node_free(&al, b);
    ASSERT_EQ_SIZE(2u, al_count(&al));
    ll_node_t *d = al_node_alloc(&al);
    ASSERT_TRUE(d != NULL);
    ASSERT_EQ_SIZE(3u, al_count(&al));
    return 0;
}

/* ---- 业务链表：add / find ---- */

static int test_add_find(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    al_init(&al, pool, 4);

    ASSERT_EQ_INT(LL_OK, al_add(&al, 100, 11));
    ASSERT_EQ_INT(LL_OK, al_add(&al, 200, 22));
    ASSERT_EQ_INT(LL_OK, al_add(&al, 300, 33));
    ASSERT_EQ_SIZE(3u, al_count(&al));

    int32_t v = 0;
    ASSERT_TRUE(al_find(&al, 200, &v));
    ASSERT_EQ_INT(22, v);
    ASSERT_TRUE(al_find(&al, 100, &v));
    ASSERT_EQ_INT(11, v);
    ASSERT_FALSE(al_find(&al, 999, &v));   /* 不存在 */
    ASSERT_TRUE(al_find(&al, 300, NULL));  /* out 可为 NULL */
    return 0;
}

/* ---- available：剩余空闲槽位随 add/remove 变化 ---- */

static int test_available(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    al_init(&al, pool, 4);
    ASSERT_EQ_SIZE(4u, al_available(&al));
    al_add(&al, 1, 0);
    al_add(&al, 2, 0);
    ASSERT_EQ_SIZE(2u, al_available(&al));
    ASSERT_EQ_SIZE(2u, al_count(&al));     /* available + count == capacity */
    al_remove(&al, 1);
    ASSERT_EQ_SIZE(3u, al_available(&al));
    return 0;
}

/* ---- update：改第一个匹配节点的 value，不新增 ---- */

static int test_update(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    al_init(&al, pool, 4);
    al_add(&al, 100, 11);
    al_add(&al, 200, 22);

    int32_t v = 0;
    ASSERT_EQ_INT(LL_OK, al_update(&al, 200, 999));
    ASSERT_EQ_SIZE(2u, al_count(&al));     /* 不新增节点 */
    ASSERT_TRUE(al_find(&al, 200, &v));
    ASSERT_EQ_INT(999, v);
    /* 不存在的码 */
    ASSERT_EQ_INT(LL_ERR_NOT_FOUND, al_update(&al, 777, 0));
    /* 重复码只更新第一个 */
    al_add(&al, 100, 55);
    ASSERT_EQ_INT(LL_OK, al_update(&al, 100, 1234));
    ASSERT_TRUE(al_find(&al, 100, &v));
    ASSERT_EQ_INT(1234, v);                 /* 第一个被改 */
    return 0;
}

/* ---- remove_all：删除所有匹配并返回数量 ---- */

static int test_remove_all(void) {
    ll_node_t pool[5];
    alarm_list_t al;
    al_init(&al, pool, 5);
    al_add(&al, 7, 1);
    al_add(&al, 9, 2);
    al_add(&al, 7, 3);
    al_add(&al, 7, 4);
    al_add(&al, 9, 5);

    ASSERT_EQ_SIZE(3u, al_remove_all(&al, 7));   /* 删了 3 个 code==7 */
    ASSERT_EQ_SIZE(2u, al_count(&al));
    ASSERT_FALSE(al_find(&al, 7, NULL));
    ASSERT_TRUE(al_find(&al, 9, NULL));
    ASSERT_EQ_SIZE(0u, al_remove_all(&al, 7));   /* 已无匹配 */
    /* 归还的节点可复用 */
    ASSERT_EQ_SIZE(3u, al_available(&al));
    return 0;
}

/* ---- for_each：按插入顺序遍历 ---- */

struct collect_ctx {
    uint16_t codes[8];
    int32_t  values[8];
    size_t   n;
};

static void collect_cb(uint16_t code, int32_t value, void *ctx) {
    struct collect_ctx *c = (struct collect_ctx *)ctx;
    if (c->n < 8) {
        c->codes[c->n] = code;
        c->values[c->n] = value;
        c->n++;
    }
}

static int test_for_each_order(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    al_init(&al, pool, 4);
    al_add(&al, 10, 100);
    al_add(&al, 20, 200);
    al_add(&al, 30, 300);

    struct collect_ctx c = {{0}, {0}, 0};
    al_for_each(&al, collect_cb, &c);
    ASSERT_EQ_SIZE(3u, c.n);
    ASSERT_EQ_INT(10, c.codes[0]);   /* 追加到尾部 => 遍历为插入顺序 */
    ASSERT_EQ_INT(20, c.codes[1]);
    ASSERT_EQ_INT(30, c.codes[2]);
    ASSERT_EQ_INT(100, c.values[0]);
    ASSERT_EQ_INT(300, c.values[2]);

    /* 删除中间后顺序仍连续 */
    al_remove(&al, 20);
    struct collect_ctx c2 = {{0}, {0}, 0};
    al_for_each(&al, collect_cb, &c2);
    ASSERT_EQ_SIZE(2u, c2.n);
    ASSERT_EQ_INT(10, c2.codes[0]);
    ASSERT_EQ_INT(30, c2.codes[1]);
    return 0;
}

/* ---- 移除：头 / 中 / 尾 ---- */

static int test_remove_middle(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    al_init(&al, pool, 4);
    al_add(&al, 1, 0);
    al_add(&al, 2, 0);
    al_add(&al, 3, 0);

    ASSERT_EQ_INT(LL_OK, al_remove(&al, 2));   /* 移除中间 */
    ASSERT_EQ_SIZE(2u, al_count(&al));
    ASSERT_FALSE(al_find(&al, 2, NULL));
    ASSERT_TRUE(al_find(&al, 1, NULL));
    ASSERT_TRUE(al_find(&al, 3, NULL));
    return 0;
}

static int test_remove_head_and_tail(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    al_init(&al, pool, 4);
    al_add(&al, 1, 0);
    al_add(&al, 2, 0);
    al_add(&al, 3, 0);

    ASSERT_EQ_INT(LL_OK, al_remove(&al, 1));   /* 头 */
    ASSERT_EQ_INT(LL_OK, al_remove(&al, 3));   /* 尾 */
    ASSERT_EQ_SIZE(1u, al_count(&al));
    ASSERT_TRUE(al_find(&al, 2, NULL));
    ASSERT_EQ_INT(LL_ERR_NOT_FOUND, al_remove(&al, 999));
    return 0;
}

/* ---- 移除后节点归还池可被复用（不 malloc） ---- */

static int test_remove_recycles(void) {
    ll_node_t pool[2];
    alarm_list_t al;
    al_init(&al, pool, 2);
    ASSERT_EQ_INT(LL_OK, al_add(&al, 1, 0));
    ASSERT_EQ_INT(LL_OK, al_add(&al, 2, 0));
    ASSERT_EQ_INT(LL_ERR_FULL, al_add(&al, 3, 0));   /* 池满 */

    ASSERT_EQ_INT(LL_OK, al_remove(&al, 1));         /* 腾出一个 */
    ASSERT_EQ_INT(LL_OK, al_add(&al, 3, 30));        /* 复用归还的节点 */
    int32_t v = 0;
    ASSERT_TRUE(al_find(&al, 3, &v));
    ASSERT_EQ_INT(30, v);
    return 0;
}

/* ---- 重复 code：remove 只删第一个 ---- */

static int test_remove_first_only(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    al_init(&al, pool, 4);
    al_add(&al, 5, 100);
    al_add(&al, 5, 200);
    ASSERT_EQ_SIZE(2u, al_count(&al));
    ASSERT_EQ_INT(LL_OK, al_remove(&al, 5));
    ASSERT_EQ_SIZE(1u, al_count(&al));   /* 还剩一个 code==5 */
    ASSERT_TRUE(al_find(&al, 5, NULL));
    return 0;
}

/* ---- reset ---- */

static int test_reset(void) {
    ll_node_t pool[4];
    alarm_list_t al;
    al_init(&al, pool, 4);
    al_add(&al, 1, 0);
    al_add(&al, 2, 0);
    al_reset(&al);
    ASSERT_EQ_SIZE(0u, al_count(&al));
    ASSERT_FALSE(al_find(&al, 1, NULL));
    /* reset 后还能正常使用 */
    ASSERT_EQ_INT(LL_OK, al_add(&al, 9, 90));
    ASSERT_EQ_SIZE(1u, al_count(&al));
    return 0;
}

/* ---- 压力测试：反复填满/清空，验证池不泄漏、不串节点 ---- */

static int test_stress_fill_drain(void) {
    ll_node_t pool[8];
    alarm_list_t al;
    al_init(&al, pool, 8);

    for (int round = 0; round < 50; round++) {
        /* 填满 */
        for (int i = 0; i < 8; i++) {
            ASSERT_EQ_INT(LL_OK, al_add(&al, (uint16_t)(i + 1), i * 10));
        }
        ASSERT_TRUE(al_is_full(&al));
        ASSERT_EQ_INT(LL_ERR_FULL, al_add(&al, 99, 0));
        ASSERT_EQ_SIZE(0u, al_available(&al));

        /* 校验内容 */
        int32_t v = 0;
        ASSERT_TRUE(al_find(&al, 5, &v));
        ASSERT_EQ_INT(40, v);

        /* 全部清空（交替用 remove 与 reset 路径） */
        if (round % 2 == 0) {
            for (int i = 0; i < 8; i++) {
                ASSERT_EQ_INT(LL_OK, al_remove(&al, (uint16_t)(i + 1)));
            }
        } else {
            al_reset(&al);
        }
        ASSERT_EQ_SIZE(0u, al_count(&al));
        ASSERT_EQ_SIZE(8u, al_available(&al));
    }
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_basic);
    RUN_TEST(test_init_null);
    RUN_TEST(test_null_safe);
    RUN_TEST(test_pool_alloc_free);
    RUN_TEST(test_add_find);
    RUN_TEST(test_available);
    RUN_TEST(test_update);
    RUN_TEST(test_remove_middle);
    RUN_TEST(test_remove_head_and_tail);
    RUN_TEST(test_remove_all);
    RUN_TEST(test_remove_recycles);
    RUN_TEST(test_remove_first_only);
    RUN_TEST(test_for_each_order);
    RUN_TEST(test_reset);
    RUN_TEST(test_stress_fill_drain);
    TEST_END();
}
