/**
 * @file test_sync_buffer.c
 * @brief Lab 6 测试套件。无需修改本文件；实现 sync_buffer.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "sync_buffer.h"
#include <pthread.h>

#define N_BYTES 2000u

/* ---- 基本（单线程）行为 ---- */

static int test_init_null(void) {
    uint8_t storage[8];
    sync_buffer_t sb;
    ASSERT_EQ_INT(SB_ERR_NULL, sb_init(NULL, storage, sizeof(storage)));
    ASSERT_EQ_INT(SB_ERR_NULL, sb_init(&sb, NULL, sizeof(storage)));
    ASSERT_EQ_INT(SB_ERR_SIZE, sb_init(&sb, storage, 0));
    ASSERT_EQ_SIZE(0u, sb_count(NULL));
    ASSERT_EQ_INT(SB_ERR_NULL, sb_put(NULL, 1));
    ASSERT_EQ_INT(SB_ERR_NULL, sb_get(NULL, NULL));
    sb_close(NULL);
    sb_destroy(NULL);
    return 0;
}

static int test_single_thread_put_get(void) {
    uint8_t storage[4];
    sync_buffer_t sb;
    ASSERT_EQ_INT(SB_OK, sb_init(&sb, storage, sizeof(storage)));

    ASSERT_EQ_INT(SB_OK, sb_put(&sb, 11));
    ASSERT_EQ_INT(SB_OK, sb_put(&sb, 22));
    ASSERT_EQ_SIZE(2u, sb_count(&sb));

    uint8_t v = 0;
    ASSERT_EQ_INT(SB_OK, sb_get(&sb, &v)); ASSERT_EQ_INT(11, v);
    ASSERT_EQ_INT(SB_OK, sb_get(&sb, &v)); ASSERT_EQ_INT(22, v);
    ASSERT_EQ_SIZE(0u, sb_count(&sb));

    sb_destroy(&sb);
    return 0;
}

static int test_close_then_get_drains(void) {
    uint8_t storage[4];
    sync_buffer_t sb;
    sb_init(&sb, storage, sizeof(storage));
    sb_put(&sb, 7);
    sb_put(&sb, 8);
    sb_close(&sb);

    /* 关闭后写被拒 */
    ASSERT_EQ_INT(SB_CLOSED, sb_put(&sb, 9));
    /* 但残留数据仍能取走 */
    uint8_t v = 0;
    ASSERT_EQ_INT(SB_OK, sb_get(&sb, &v)); ASSERT_EQ_INT(7, v);
    ASSERT_EQ_INT(SB_OK, sb_get(&sb, &v)); ASSERT_EQ_INT(8, v);
    /* 取完后返回 SB_CLOSED */
    ASSERT_EQ_INT(SB_CLOSED, sb_get(&sb, &v));

    sb_destroy(&sb);
    return 0;
}

/* ---- 多线程：单生产者 / 单消费者，小缓冲强制阻塞 ---- */

typedef struct {
    sync_buffer_t *sb;
    int            ok;   /* 消费者用于标记顺序是否正确 */
    size_t         got;  /* 消费者实际取到的字节数     */
} consumer_arg_t;

static void *producer_main(void *p) {
    sync_buffer_t *sb = (sync_buffer_t *)p;
    for (unsigned i = 0; i < N_BYTES; i++) {
        if (sb_put(sb, (uint8_t)(i & 0xFF)) != SB_OK) break;
    }
    sb_close(sb);
    return NULL;
}

static void *consumer_main(void *p) {
    consumer_arg_t *arg = (consumer_arg_t *)p;
    arg->ok = 1;
    arg->got = 0;
    uint8_t v = 0;
    for (;;) {
        sb_status_t st = sb_get(arg->sb, &v);
        if (st == SB_CLOSED) break;
        if (st != SB_OK) { arg->ok = 0; break; }
        /* 数据应严格按 0,1,2,... mod 256 的顺序到达 */
        if (v != (uint8_t)(arg->got & 0xFF)) arg->ok = 0;
        arg->got++;
    }
    return NULL;
}

static int test_spsc_blocking(void) {
    uint8_t storage[4];   /* 故意很小，逼出"满/空阻塞" */
    sync_buffer_t sb;
    sb_init(&sb, storage, sizeof(storage));

    consumer_arg_t arg = { &sb, 0, 0 };
    pthread_t prod, cons;
    ASSERT_EQ_INT(0, pthread_create(&cons, NULL, consumer_main, &arg));
    ASSERT_EQ_INT(0, pthread_create(&prod, NULL, producer_main, &sb));
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    ASSERT_TRUE(arg.ok == 1);
    ASSERT_EQ_SIZE((size_t)N_BYTES, arg.got);

    sb_destroy(&sb);
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_null);
    RUN_TEST(test_single_thread_put_get);
    RUN_TEST(test_close_then_get_drains);
    RUN_TEST(test_spsc_blocking);
    TEST_END();
}
