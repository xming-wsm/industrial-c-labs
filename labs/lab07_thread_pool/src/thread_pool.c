/**
 * @file thread_pool.c
 * @brief Lab 7 - 线程池 + 任务队列实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_thread_pool.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab07_thread_pool.md）：
 *   - 任务队列就是 Lab2 的环形数组：入队写 (qhead+qcount)%qcap，
 *     出队取 qhead 并前进；用 not_empty/not_full 两个条件变量阻塞。
 *   - 工作线程主循环 worker_main：
 *       lock;
 *       while (qcount==0 && !shutdown) cond_wait(not_empty);
 *       if (qcount==0 && shutdown) { unlock; 退出线程; }
 *       取出一个任务; qcount--; cond_signal(not_full);
 *       unlock;
 *       task.fn(task.arg);   // 注意：执行任务时不要持锁！
 *   - tp_submit：lock; while(满 && !shutdown) cond_wait(not_full);
 *       若 shutdown -> 返回 TP_ERR_SHUTDOWN；否则入队、signal(not_empty)。
 *   - tp_shutdown：lock; shutdown=true; broadcast 两个 cond; unlock;
 *       join 所有线程；destroy mutex/cond。
 *   - 用 while 重判条件，防止虚假唤醒；执行任务时务必先解锁。
 */
#include "thread_pool.h"

/* 建议：把工作线程主循环写成一个 static 函数。
 * static void *worker_main(void *arg) { ... } */

tp_status_t tp_init(thread_pool_t *tp,
                    pthread_t *worker_storage, size_t nworkers,
                    tp_task_t *queue_storage, size_t qcap) {
    /* TODO:
     *   1. tp/worker_storage/queue_storage 为 NULL -> TP_ERR_NULL
     *   2. nworkers==0 || qcap==0 -> TP_ERR_ARG
     *   3. 绑定字段、清零队列游标、shutdown=false、init mutex/cond
     *   4. pthread_create 启动 nworkers 个工作线程（都跑 worker_main，arg=tp）
     *   -> TP_OK
     */
    (void)tp;
    (void)worker_storage;
    (void)nworkers;
    (void)queue_storage;
    (void)qcap;
    return TP_ERR_NULL; /* 占位：实现前测试应当失败 */
}

tp_status_t tp_submit(thread_pool_t *tp, tp_task_fn fn, void *arg) {
    /* TODO:
     *   tp/fn 为 NULL -> TP_ERR_NULL；
     *   先做快速判断：if (tp->shutdown) return TP_ERR_SHUTDOWN;
     *     （关闭后 mutex 可能已被销毁，必须在加锁前就挡掉新任务）
     *   lock; while(qcount==qcap && !shutdown) cond_wait(not_full);
     *   若 shutdown -> unlock 返回 TP_ERR_SHUTDOWN；
     *   否则写入 (qhead+qcount)%qcap，qcount++，cond_signal(not_empty)，unlock，返回 TP_OK。
     */
    (void)tp;
    (void)fn;
    (void)arg;
    return TP_ERR_NULL;
}

void tp_shutdown(thread_pool_t *tp) {
    /* TODO:
     *   tp 为 NULL -> 返回；
     *   lock; shutdown=true; cond_broadcast(not_empty); cond_broadcast(not_full); unlock;
     *   for 每个 worker：pthread_join；
     *   destroy mutex 与两个 cond。
     */
    (void)tp;
}

size_t tp_pending(thread_pool_t *tp) {
    /* TODO: NULL 返回 0；否则加锁读 qcount 再解锁返回。 */
    (void)tp;
    return 0;
}
