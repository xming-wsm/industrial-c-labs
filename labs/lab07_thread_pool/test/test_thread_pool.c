/**
 * @file test_thread_pool.c
 * @brief Lab 7 测试套件。无需修改本文件；实现 thread_pool.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "thread_pool.h"
#include <pthread.h>

/* 共享计数器：每个任务把它 +1（加锁保护）。 */
typedef struct {
    pthread_mutex_t lock;
    long            sum;     /* 累加所有任务的 arg 值 */
    int             count;   /* 执行过的任务数        */
} counter_t;

static void task_add(void *arg) {
    /* arg 实际是 counter_t*，但每个任务要加的值通过下面的技巧传递。
     * 这里简化：所有任务都对同一个 counter 执行 +1 与 sum += 1。 */
    counter_t *c = (counter_t *)arg;
    pthread_mutex_lock(&c->lock);
    c->sum += 1;
    c->count += 1;
    pthread_mutex_unlock(&c->lock);
}

static int test_init_null(void) {
    pthread_t workers[2];
    tp_task_t queue[4];
    thread_pool_t tp;
    ASSERT_EQ_INT(TP_ERR_NULL, tp_init(NULL, workers, 2, queue, 4));
    ASSERT_EQ_INT(TP_ERR_NULL, tp_init(&tp, NULL, 2, queue, 4));
    ASSERT_EQ_INT(TP_ERR_NULL, tp_init(&tp, workers, 2, NULL, 4));
    ASSERT_EQ_INT(TP_ERR_ARG, tp_init(&tp, workers, 0, queue, 4));
    ASSERT_EQ_INT(TP_ERR_ARG, tp_init(&tp, workers, 2, queue, 0));
    ASSERT_EQ_INT(TP_ERR_NULL, tp_submit(NULL, task_add, NULL));
    ASSERT_EQ_SIZE(0u, tp_pending(NULL));
    tp_shutdown(NULL);
    return 0;
}

/* 提交少量任务，关闭后校验全部执行。 */
static int test_run_few(void) {
    pthread_t workers[3];
    tp_task_t queue[4];
    thread_pool_t tp;
    counter_t c;
    pthread_mutex_init(&c.lock, NULL);
    c.sum = 0;
    c.count = 0;

    ASSERT_EQ_INT(TP_OK, tp_init(&tp, workers, 3, queue, 4));
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ_INT(TP_OK, tp_submit(&tp, task_add, &c));
    }
    tp_shutdown(&tp);   /* 关闭前会把已入队任务全部跑完 */

    ASSERT_EQ_INT(10, c.count);
    ASSERT_EQ_INT(10, (int)c.sum);
    pthread_mutex_destroy(&c.lock);
    return 0;
}

/* 大量任务 + 小队列：逼出 submit 阻塞（背压），校验无丢失。 */
static int test_many_with_backpressure(void) {
    pthread_t workers[4];
    tp_task_t queue[8];     /* 队列小，提交会被阻塞 */
    thread_pool_t tp;
    counter_t c;
    pthread_mutex_init(&c.lock, NULL);
    c.sum = 0;
    c.count = 0;

    const int N = 5000;
    ASSERT_EQ_INT(TP_OK, tp_init(&tp, workers, 4, queue, 8));
    for (int i = 0; i < N; i++) {
        ASSERT_EQ_INT(TP_OK, tp_submit(&tp, task_add, &c));
    }
    tp_shutdown(&tp);

    ASSERT_EQ_INT(N, c.count);     /* 一个不少，一个不多 */
    ASSERT_EQ_INT(N, (int)c.sum);
    pthread_mutex_destroy(&c.lock);
    return 0;
}

/* 关闭后再提交应被拒绝。 */
static int test_submit_after_shutdown(void) {
    pthread_t workers[2];
    tp_task_t queue[4];
    thread_pool_t tp;
    counter_t c;
    pthread_mutex_init(&c.lock, NULL);
    c.sum = 0;
    c.count = 0;

    tp_init(&tp, workers, 2, queue, 4);
    tp_submit(&tp, task_add, &c);
    tp_shutdown(&tp);
    /* 关闭后控制块仍可读；新提交应被拒（这里复用已关闭的 tp 仅做返回值检查）。 */
    ASSERT_EQ_INT(TP_ERR_SHUTDOWN, tp_submit(&tp, task_add, &c));

    pthread_mutex_destroy(&c.lock);
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_null);
    RUN_TEST(test_run_few);
    RUN_TEST(test_many_with_backpressure);
    RUN_TEST(test_submit_after_shutdown);
    TEST_END();
}
