/**
 * @file thread_pool.c
 * @brief Lab 7 - 线程池 + 任务队列（待你完成）
 *
 * 详见 docs/lab07_thread_pool.md；测试：xmake lab7 test
 */
#include "thread_pool.h"

tp_status_t tp_init(thread_pool_t *tp,
                    pthread_t *worker_storage, size_t nworkers,
                    tp_task_t *queue_storage, size_t qcap) {
    (void)tp;
    (void)worker_storage;
    (void)nworkers;
    (void)queue_storage;
    (void)qcap;
    return TP_ERR_NULL;
}

tp_status_t tp_submit(thread_pool_t *tp, tp_task_fn fn, void *arg) {
    (void)tp;
    (void)fn;
    (void)arg;
    return TP_ERR_NULL;
}

tp_status_t tp_try_submit(thread_pool_t *tp, tp_task_fn fn, void *arg) {
    (void)tp;
    (void)fn;
    (void)arg;
    return TP_ERR_NULL;
}

void tp_shutdown(thread_pool_t *tp) {
    (void)tp;
}

size_t tp_pending(thread_pool_t *tp) {
    (void)tp;
    return 0;
}

size_t tp_worker_count(const thread_pool_t *tp) {
    (void)tp;
    return 0;
}

size_t tp_capacity(const thread_pool_t *tp) {
    (void)tp;
    return 0;
}
