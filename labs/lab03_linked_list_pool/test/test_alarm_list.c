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
    ASSERT_FALSE(al_is_full(NULL));
    ASSERT_TRUE(al_node_alloc(NULL) == NULL);
    al_node_free(NULL, NULL);
    ASSERT_EQ_INT(LL_ERR_NULL, al_add(NULL, 1, 2));
    ASSERT_FALSE(al_find(NULL, 1, NULL));
    ASSERT_EQ_INT(LL_ERR_NULL, al_remove(NULL, 1));
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

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_basic);
    RUN_TEST(test_init_null);
    RUN_TEST(test_null_safe);
    RUN_TEST(test_pool_alloc_free);
    RUN_TEST(test_add_find);
    RUN_TEST(test_remove_middle);
    RUN_TEST(test_remove_head_and_tail);
    RUN_TEST(test_remove_recycles);
    RUN_TEST(test_remove_first_only);
    RUN_TEST(test_reset);
    TEST_END();
}
