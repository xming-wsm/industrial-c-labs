/**
 * @file test_thread_pool.c
 * @brief Lab 7 测试套件。无需修改本文件；实现 thread_pool.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "thread_pool.h"
#include <pthread.h>
#include <unistd.h>

/* 共享计数器：每个任务把它 +1（加锁保护）。 */
typedef struct {
    pthread_mutex_t lock;
    long            sum;     /* 累加所有任务的 arg 值 */
    int             count;   /* 执行过的任务数        */
} counter_t;

/* 一个"闸门"：任务会卡在这里，直到测试主动放行，便于制造"队列满"。 */
typedef struct {
    pthread_mutex_t m;
    pthread_cond_t  cv;
    int             open;
} gate_t;

static void task_wait_gate(void *arg) {
    gate_t *g = (gate_t *)arg;
    pthread_mutex_lock(&g->m);
    while (!g->open) {
        pthread_cond_wait(&g->cv, &g->m);
    }
    pthread_mutex_unlock(&g->m);
}

static void gate_open(gate_t *g) {
    pthread_mutex_lock(&g->m);
    g->open = 1;
    pthread_cond_broadcast(&g->cv);
    pthread_mutex_unlock(&g->m);
}

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
    ASSERT_EQ_INT(TP_ERR_NULL, tp_try_submit(NULL, task_add, NULL));
    ASSERT_EQ_SIZE(0u, tp_pending(NULL));
    ASSERT_EQ_SIZE(0u, tp_worker_count(NULL));
    ASSERT_EQ_SIZE(0u, tp_capacity(NULL));
    tp_shutdown(NULL);
    return 0;
}

/* 配置信息：worker 数与队列容量可读回。 */
static int test_config_getters(void) {
    pthread_t workers[3];
    tp_task_t queue[5];
    thread_pool_t tp;
    ASSERT_EQ_INT(TP_OK, tp_init(&tp, workers, 3, queue, 5));
    ASSERT_EQ_SIZE(3u, tp_worker_count(&tp));
    ASSERT_EQ_SIZE(5u, tp_capacity(&tp));
    tp_shutdown(&tp);
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

/* 非阻塞提交：占满唯一 worker 后填满队列，try_submit 应返回 FULL。 */
static int test_try_submit_full(void) {
    pthread_t workers[1];
    tp_task_t queue[2];
    thread_pool_t tp;
    gate_t gate = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0 };
    counter_t c;
    pthread_mutex_init(&c.lock, NULL);
    c.sum = 0;
    c.count = 0;

    ASSERT_EQ_INT(TP_OK, tp_init(&tp, workers, 1, queue, 2));

    /* 让唯一的 worker 卡在闸门上 */
    ASSERT_EQ_INT(TP_OK, tp_submit(&tp, task_wait_gate, &gate));
    /* 等待 worker 取走这个任务（pending 归零，worker 进入等待） */
    for (int i = 0; i < 1000 && tp_pending(&tp) != 0; i++) {
        usleep(1000);
    }

    /* worker 被占住，队列可被填满到容量 2 */
    ASSERT_EQ_INT(TP_OK, tp_try_submit(&tp, task_add, &c));
    ASSERT_EQ_INT(TP_OK, tp_try_submit(&tp, task_add, &c));
    /* 再提交应立即返回 FULL，而不是阻塞 */
    ASSERT_EQ_INT(TP_ERR_FULL, tp_try_submit(&tp, task_add, &c));

    /* 放行闸门，剩余任务被执行 */
    gate_open(&gate);
    tp_shutdown(&tp);

    ASSERT_EQ_INT(2, c.count);   /* 两个排队任务都跑完 */
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
    ASSERT_EQ_INT(TP_ERR_SHUTDOWN, tp_try_submit(&tp, task_add, &c));

    pthread_mutex_destroy(&c.lock);
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_null);
    RUN_TEST(test_config_getters);
    RUN_TEST(test_run_few);
    RUN_TEST(test_many_with_backpressure);
    RUN_TEST(test_try_submit_full);
    RUN_TEST(test_submit_after_shutdown);
    TEST_END();
}
