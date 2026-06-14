/**
 * @file test_ring_buffer.c
 * @brief Lab 1 测试套件。无需修改本文件；实现 ring_buffer.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "ring_buffer.h"

/* ---- 初始化与参数校验 ---- */

static int test_init_basic(void) {
    uint8_t storage[8];
    ring_buffer_t rb;
    ASSERT_EQ_INT(RB_OK, rb_init(&rb, storage, sizeof(storage)));
    ASSERT_EQ_SIZE(8u, rb_capacity(&rb));
    ASSERT_EQ_SIZE(0u, rb_count(&rb));
    ASSERT_TRUE(rb_is_empty(&rb));
    ASSERT_FALSE(rb_is_full(&rb));
    return 0;
}

static int test_init_null_and_size(void) {
    uint8_t storage[8];
    ring_buffer_t rb;
    ASSERT_EQ_INT(RB_ERR_NULL, rb_init(NULL, storage, sizeof(storage)));
    ASSERT_EQ_INT(RB_ERR_NULL, rb_init(&rb, NULL, sizeof(storage)));
    ASSERT_EQ_INT(RB_ERR_SIZE, rb_init(&rb, storage, 0));
    return 0;
}

static int test_null_safe_queries(void) {
    /* 所有查询函数对 NULL 都应安全。 */
    ASSERT_TRUE(rb_is_empty(NULL));
    ASSERT_FALSE(rb_is_full(NULL));
    ASSERT_EQ_SIZE(0u, rb_count(NULL));
    ASSERT_EQ_SIZE(0u, rb_capacity(NULL));
    ASSERT_EQ_INT(RB_ERR_NULL, rb_push(NULL, 0x55));
    ASSERT_EQ_INT(RB_ERR_NULL, rb_pop(NULL, NULL));
    ASSERT_EQ_SIZE(0u, rb_write(NULL, (const uint8_t *)"x", 1));
    ASSERT_EQ_SIZE(0u, rb_read(NULL, (uint8_t *)"x", 1));
    rb_reset(NULL); /* 不应崩溃 */
    return 0;
}

/* ---- 单字节 push / pop ---- */

static int test_push_pop_fifo_order(void) {
    uint8_t storage[4];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));

    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 10));
    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 20));
    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 30));
    ASSERT_EQ_SIZE(3u, rb_count(&rb));

    uint8_t v = 0;
    ASSERT_EQ_INT(RB_OK, rb_pop(&rb, &v));
    ASSERT_EQ_INT(10, v);
    ASSERT_EQ_INT(RB_OK, rb_pop(&rb, &v));
    ASSERT_EQ_INT(20, v);
    ASSERT_EQ_INT(RB_OK, rb_pop(&rb, &v));
    ASSERT_EQ_INT(30, v);
    ASSERT_TRUE(rb_is_empty(&rb));
    return 0;
}

static int test_pop_null_out_discards(void) {
    uint8_t storage[4];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));
    rb_push(&rb, 99);
    /* out 为 NULL 表示丢弃该字节，但 count 仍应递减。 */
    ASSERT_EQ_INT(RB_OK, rb_pop(&rb, NULL));
    ASSERT_EQ_SIZE(0u, rb_count(&rb));
    return 0;
}

/* ---- 满 / 空边界 ---- */

static int test_fill_until_full(void) {
    uint8_t storage[3];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));

    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 1));
    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 2));
    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 3));
    ASSERT_TRUE(rb_is_full(&rb));
    ASSERT_EQ_SIZE(3u, rb_count(&rb));
    /* 满了之后再写应被拒绝。 */
    ASSERT_EQ_INT(RB_ERR_FULL, rb_push(&rb, 4));
    ASSERT_EQ_SIZE(3u, rb_count(&rb));
    return 0;
}

static int test_pop_empty(void) {
    uint8_t storage[4];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));
    uint8_t v = 0;
    ASSERT_EQ_INT(RB_ERR_EMPTY, rb_pop(&rb, &v));
    return 0;
}

/* ---- 环绕（wrap-around）：读写游标越过末尾后仍正确 ---- */

static int test_wrap_around(void) {
    uint8_t storage[4];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));

    /* 先写满、读出 2 个，使 head 回绕，再继续写，验证环绕正确。 */
    for (uint8_t i = 1; i <= 4; i++) {
        ASSERT_EQ_INT(RB_OK, rb_push(&rb, i)); /* 1,2,3,4 */
    }
    uint8_t v = 0;
    ASSERT_EQ_INT(RB_OK, rb_pop(&rb, &v));
    ASSERT_EQ_INT(1, v);
    ASSERT_EQ_INT(RB_OK, rb_pop(&rb, &v));
    ASSERT_EQ_INT(2, v);

    /* 现在写入会绕回缓冲区头部。 */
    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 5));
    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 6));
    ASSERT_TRUE(rb_is_full(&rb));

    /* 顺序应为 3,4,5,6 */
    uint8_t expect[4] = {3, 4, 5, 6};
    for (int i = 0; i < 4; i++) {
        ASSERT_EQ_INT(RB_OK, rb_pop(&rb, &v));
        ASSERT_EQ_INT(expect[i], v);
    }
    ASSERT_TRUE(rb_is_empty(&rb));
    return 0;
}

/* ---- 批量 write / read ---- */

static int test_bulk_write_read(void) {
    uint8_t storage[8];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));

    const uint8_t in[5] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4};
    ASSERT_EQ_SIZE(5u, rb_write(&rb, in, 5));
    ASSERT_EQ_SIZE(5u, rb_count(&rb));

    uint8_t out[5] = {0};
    ASSERT_EQ_SIZE(5u, rb_read(&rb, out, 5));
    ASSERT_EQ_MEM(in, out, 5);
    ASSERT_TRUE(rb_is_empty(&rb));
    return 0;
}

static int test_bulk_write_partial_when_full(void) {
    uint8_t storage[4];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));

    const uint8_t in[6] = {1, 2, 3, 4, 5, 6};
    /* 容量 4，写 6 个只能写进 4 个。 */
    ASSERT_EQ_SIZE(4u, rb_write(&rb, in, 6));
    ASSERT_TRUE(rb_is_full(&rb));

    uint8_t out[6] = {0};
    /* 只能读出 4 个。 */
    ASSERT_EQ_SIZE(4u, rb_read(&rb, out, 6));
    const uint8_t expect[4] = {1, 2, 3, 4};
    ASSERT_EQ_MEM(expect, out, 4);
    return 0;
}

static int test_bulk_wrap(void) {
    uint8_t storage[4];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));

    /* 制造一个非零起点：写 3、读 3，使 head/tail 都前进到 3。 */
    const uint8_t seed[3] = {7, 8, 9};
    ASSERT_EQ_SIZE(3u, rb_write(&rb, seed, 3));
    uint8_t tmp[3] = {0};
    ASSERT_EQ_SIZE(3u, rb_read(&rb, tmp, 3));
    ASSERT_TRUE(rb_is_empty(&rb));

    /* 现在批量写 4 个会跨越缓冲区末尾。 */
    const uint8_t in[4] = {11, 12, 13, 14};
    ASSERT_EQ_SIZE(4u, rb_write(&rb, in, 4));
    uint8_t out[4] = {0};
    ASSERT_EQ_SIZE(4u, rb_read(&rb, out, 4));
    ASSERT_EQ_MEM(in, out, 4);
    return 0;
}

/* ---- reset ---- */

static int test_reset(void) {
    uint8_t storage[4];
    ring_buffer_t rb;
    rb_init(&rb, storage, sizeof(storage));
    rb_push(&rb, 1);
    rb_push(&rb, 2);
    rb_reset(&rb);
    ASSERT_TRUE(rb_is_empty(&rb));
    ASSERT_EQ_SIZE(0u, rb_count(&rb));
    ASSERT_EQ_SIZE(4u, rb_capacity(&rb)); /* 容量不变 */
    /* reset 之后还能正常使用。 */
    ASSERT_EQ_INT(RB_OK, rb_push(&rb, 42));
    uint8_t v = 0;
    ASSERT_EQ_INT(RB_OK, rb_pop(&rb, &v));
    ASSERT_EQ_INT(42, v);
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_basic);
    RUN_TEST(test_init_null_and_size);
    RUN_TEST(test_null_safe_queries);
    RUN_TEST(test_push_pop_fifo_order);
    RUN_TEST(test_pop_null_out_discards);
    RUN_TEST(test_fill_until_full);
    RUN_TEST(test_pop_empty);
    RUN_TEST(test_wrap_around);
    RUN_TEST(test_bulk_write_read);
    RUN_TEST(test_bulk_write_partial_when_full);
    RUN_TEST(test_bulk_wrap);
    RUN_TEST(test_reset);
    TEST_END();
}
