/**
 * @file test_command_queue.c
 * @brief Lab 2 测试套件。无需修改本文件；实现 command_queue.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "command_queue.h"

static cmd_t mk(uint16_t id, int32_t arg) {
    cmd_t c;
    c.id = id;
    c.arg = arg;
    return c;
}

/* ---- 初始化与参数校验 ---- */

static int test_init_basic(void) {
    cmd_t storage[4];
    command_queue_t cq;
    ASSERT_EQ_INT(CQ_OK, cq_init(&cq, storage, 4));
    ASSERT_EQ_SIZE(4u, cq_capacity(&cq));
    ASSERT_EQ_SIZE(0u, cq_count(&cq));
    ASSERT_TRUE(cq_is_empty(&cq));
    ASSERT_FALSE(cq_is_full(&cq));
    return 0;
}

static int test_init_null_and_size(void) {
    cmd_t storage[4];
    command_queue_t cq;
    ASSERT_EQ_INT(CQ_ERR_NULL, cq_init(NULL, storage, 4));
    ASSERT_EQ_INT(CQ_ERR_NULL, cq_init(&cq, NULL, 4));
    ASSERT_EQ_INT(CQ_ERR_SIZE, cq_init(&cq, storage, 0));
    return 0;
}

static int test_null_safe(void) {
    cmd_t out;
    ASSERT_TRUE(cq_is_empty(NULL));
    ASSERT_FALSE(cq_is_full(NULL));
    ASSERT_EQ_SIZE(0u, cq_count(NULL));
    ASSERT_EQ_SIZE(0u, cq_capacity(NULL));
    ASSERT_EQ_INT(CQ_ERR_NULL, cq_push_back(NULL, mk(1, 2)));
    ASSERT_EQ_INT(CQ_ERR_NULL, cq_push_front(NULL, mk(1, 2)));
    ASSERT_EQ_INT(CQ_ERR_NULL, cq_pop_front(NULL, &out));
    ASSERT_EQ_INT(CQ_ERR_NULL, cq_pop_back(NULL, &out));
    ASSERT_EQ_INT(CQ_ERR_NULL, cq_front(NULL, &out));
    ASSERT_EQ_INT(CQ_ERR_NULL, cq_back(NULL, &out));
    cq_reset(NULL); /* 不应崩溃 */
    return 0;
}

/* ---- FIFO：push_back / pop_front ---- */

static int test_fifo_order(void) {
    cmd_t storage[4];
    command_queue_t cq;
    cq_init(&cq, storage, 4);

    ASSERT_EQ_INT(CQ_OK, cq_push_back(&cq, mk(10, 100)));
    ASSERT_EQ_INT(CQ_OK, cq_push_back(&cq, mk(20, 200)));
    ASSERT_EQ_INT(CQ_OK, cq_push_back(&cq, mk(30, 300)));
    ASSERT_EQ_SIZE(3u, cq_count(&cq));

    cmd_t out;
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(10, out.id);
    ASSERT_EQ_INT(100, out.arg);
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(20, out.id);
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(30, out.id);
    ASSERT_TRUE(cq_is_empty(&cq));
    return 0;
}

/* ---- 插队：push_front 让命令排到队首 ---- */

static int test_push_front_priority(void) {
    cmd_t storage[4];
    command_queue_t cq;
    cq_init(&cq, storage, 4);

    cq_push_back(&cq, mk(1, 0));
    cq_push_back(&cq, mk(2, 0));
    /* 急停插队到最前 */
    ASSERT_EQ_INT(CQ_OK, cq_push_front(&cq, mk(99, -1)));

    cmd_t out;
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(99, out.id);     /* 先出来的是急停 */
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(1, out.id);
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(2, out.id);
    return 0;
}

/* ---- pop_back：撤销最近入队 ---- */

static int test_pop_back(void) {
    cmd_t storage[4];
    command_queue_t cq;
    cq_init(&cq, storage, 4);
    cq_push_back(&cq, mk(1, 0));
    cq_push_back(&cq, mk(2, 0));
    cq_push_back(&cq, mk(3, 0));

    cmd_t out;
    ASSERT_EQ_INT(CQ_OK, cq_pop_back(&cq, &out));
    ASSERT_EQ_INT(3, out.id);      /* 最后入队的先被撤销 */
    ASSERT_EQ_SIZE(2u, cq_count(&cq));
    /* 队首仍是 1 */
    ASSERT_EQ_INT(CQ_OK, cq_front(&cq, &out));
    ASSERT_EQ_INT(1, out.id);
    return 0;
}

/* ---- peek：front / back ---- */

static int test_peek(void) {
    cmd_t storage[4];
    command_queue_t cq;
    cq_init(&cq, storage, 4);
    cq_push_back(&cq, mk(7, 70));
    cq_push_back(&cq, mk(8, 80));

    cmd_t out;
    ASSERT_EQ_INT(CQ_OK, cq_front(&cq, &out));
    ASSERT_EQ_INT(7, out.id);
    ASSERT_EQ_INT(CQ_OK, cq_back(&cq, &out));
    ASSERT_EQ_INT(8, out.id);
    /* peek 不改变数量 */
    ASSERT_EQ_SIZE(2u, cq_count(&cq));
    return 0;
}

/* ---- 满 / 空边界 ---- */

static int test_full_and_empty(void) {
    cmd_t storage[2];
    command_queue_t cq;
    cq_init(&cq, storage, 2);

    ASSERT_EQ_INT(CQ_OK, cq_push_back(&cq, mk(1, 0)));
    ASSERT_EQ_INT(CQ_OK, cq_push_front(&cq, mk(2, 0)));
    ASSERT_TRUE(cq_is_full(&cq));
    ASSERT_EQ_INT(CQ_ERR_FULL, cq_push_back(&cq, mk(3, 0)));
    ASSERT_EQ_INT(CQ_ERR_FULL, cq_push_front(&cq, mk(3, 0)));

    cmd_t out;
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_TRUE(cq_is_empty(&cq));
    ASSERT_EQ_INT(CQ_ERR_EMPTY, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(CQ_ERR_EMPTY, cq_pop_back(&cq, &out));
    ASSERT_EQ_INT(CQ_ERR_EMPTY, cq_front(&cq, &out));
    ASSERT_EQ_INT(CQ_ERR_EMPTY, cq_back(&cq, &out));
    return 0;
}

/* ---- 环绕：混合 push_front / push_back 跨越数组边界 ---- */

static int test_wrap_mixed(void) {
    cmd_t storage[4];
    command_queue_t cq;
    cq_init(&cq, storage, 4);

    /* 先 push_front 让 head 回绕到数组末尾附近 */
    ASSERT_EQ_INT(CQ_OK, cq_push_front(&cq, mk(1, 0)));  /* head 回绕 */
    ASSERT_EQ_INT(CQ_OK, cq_push_back(&cq, mk(2, 0)));
    ASSERT_EQ_INT(CQ_OK, cq_push_front(&cq, mk(0, 0)));
    ASSERT_EQ_INT(CQ_OK, cq_push_back(&cq, mk(3, 0)));
    ASSERT_TRUE(cq_is_full(&cq));

    /* 期望出队顺序：0,1,2,3 */
    cmd_t out;
    for (uint16_t i = 0; i < 4; i++) {
        ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
        ASSERT_EQ_INT(i, out.id);
    }
    ASSERT_TRUE(cq_is_empty(&cq));
    return 0;
}

/* ---- pop 的 out 可为 NULL（丢弃） ---- */

static int test_pop_null_out(void) {
    cmd_t storage[4];
    command_queue_t cq;
    cq_init(&cq, storage, 4);
    cq_push_back(&cq, mk(1, 0));
    cq_push_back(&cq, mk(2, 0));
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, NULL));
    ASSERT_EQ_INT(CQ_OK, cq_pop_back(&cq, NULL));
    ASSERT_TRUE(cq_is_empty(&cq));
    return 0;
}

/* ---- reset ---- */

static int test_reset(void) {
    cmd_t storage[4];
    command_queue_t cq;
    cq_init(&cq, storage, 4);
    cq_push_back(&cq, mk(1, 0));
    cq_push_back(&cq, mk(2, 0));
    cq_reset(&cq);
    ASSERT_TRUE(cq_is_empty(&cq));
    ASSERT_EQ_SIZE(0u, cq_count(&cq));
    ASSERT_EQ_SIZE(4u, cq_capacity(&cq));
    ASSERT_EQ_INT(CQ_OK, cq_push_back(&cq, mk(42, 0)));
    cmd_t out;
    ASSERT_EQ_INT(CQ_OK, cq_pop_front(&cq, &out));
    ASSERT_EQ_INT(42, out.id);
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_basic);
    RUN_TEST(test_init_null_and_size);
    RUN_TEST(test_null_safe);
    RUN_TEST(test_fifo_order);
    RUN_TEST(test_push_front_priority);
    RUN_TEST(test_pop_back);
    RUN_TEST(test_peek);
    RUN_TEST(test_full_and_empty);
    RUN_TEST(test_wrap_mixed);
    RUN_TEST(test_pop_null_out);
    RUN_TEST(test_reset);
    TEST_END();
}
