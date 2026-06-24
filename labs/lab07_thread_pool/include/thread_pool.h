/**
 * @file thread_pool.h
 * @brief Lab 7 - 线程池 + 任务队列 API 声明。
 *
 * 工业背景
 * --------
 * 注塑机控制器有很多"后台杂活"：把数据写日志文件、上报 MES、分发告警、
 * 生成趋势图……这些活儿耗时不定，又不能阻塞实时控制循环。
 * 每来一件就新建一个线程？线程创建/销毁开销大、数量不可控，会拖垮系统。
 *
 * 工业界标准做法是**线程池**：预先开好固定数量的工作线程，所有任务先进
 * 一个**任务队列**，空闲线程从队列里取任务执行。好处：线程数可控、复用
 * 线程省去频繁创建开销、任务排队削峰。
 *
 * 本关把 Lab2 的队列 + Lab6 的"加锁阻塞队列"组合起来，做一个最小线程池。
 *
 * 设计约束
 * --------
 *   1. 工作线程数组与任务队列存储区都由调用者提供（不 malloc 这两块）。
 *   2. 任务队列满时 submit 阻塞等待（有界队列，背压）。
 *   3. tp_shutdown 要"优雅关闭"：把已入队的任务跑完，再 join 所有线程。
 */
#ifndef LAB07_THREAD_POOL_H
#define LAB07_THREAD_POOL_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 操作返回状态码。 */
typedef enum {
    TP_OK = 0,          /**< 成功                       */
    TP_ERR_NULL,        /**< 传入了 NULL 指针           */
    TP_ERR_ARG,         /**< 参数非法（线程数/队列容量为 0） */
    TP_ERR_SHUTDOWN,    /**< 线程池已关闭，拒绝新任务   */
    TP_ERR_FULL         /**< 非阻塞提交：队列当前已满（try_submit） */
} tp_status_t;

/** 任务函数：池里的工作线程会以 fn(arg) 的形式执行它。 */
typedef void (*tp_task_fn)(void *arg);

/** 一个任务 = 函数 + 参数。 */
typedef struct {
    tp_task_fn fn;
    void      *arg;
} tp_task_t;

/**
 * 线程池控制块。
 * 字段公开仅为方便测试与教学，正常使用请只通过 API 访问。
 */
typedef struct {
    pthread_t      *workers;    /**< 工作线程数组（调用者提供） */
    size_t          nworkers;   /**< 工作线程数                 */
    tp_task_t      *queue;      /**< 任务队列存储（调用者提供） */
    size_t          qcap;       /**< 任务队列容量               */
    size_t          qhead;      /**< 队首下标                   */
    size_t          qcount;     /**< 队列中待执行任务数         */
    bool            shutdown;   /**< 是否已请求关闭             */
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;  /**< "队列有任务"               */
    pthread_cond_t  not_full;   /**< "队列有空位"               */
} thread_pool_t;

/**
 * 创建并启动线程池。
 *
 * @param tp              控制块。
 * @param worker_storage  pthread_t 数组（至少 nworkers 个）。
 * @param nworkers        工作线程数，必须 > 0。
 * @param queue_storage   tp_task_t 数组（至少 qcap 个）。
 * @param qcap            任务队列容量，必须 > 0。
 * @return TP_OK；任一指针 NULL -> TP_ERR_NULL；nworkers/qcap 为 0 -> TP_ERR_ARG。
 */
tp_status_t tp_init(thread_pool_t *tp,
                    pthread_t *worker_storage, size_t nworkers,
                    tp_task_t *queue_storage, size_t qcap);

/**
 * 提交一个任务；队列满时阻塞等待，直到有空位或线程池被关闭。
 * @return TP_OK；tp/fn 为 NULL -> TP_ERR_NULL；已关闭 -> TP_ERR_SHUTDOWN。
 */
tp_status_t tp_submit(thread_pool_t *tp, tp_task_fn fn, void *arg);

/**
 * 非阻塞提交：队列满时不等待，立即返回 TP_ERR_FULL。
 * @return TP_OK；TP_ERR_FULL（队列已满）；tp/fn 为 NULL -> TP_ERR_NULL；
 *         已关闭 -> TP_ERR_SHUTDOWN。
 */
tp_status_t tp_try_submit(thread_pool_t *tp, tp_task_fn fn, void *arg);

/**
 * 优雅关闭：把队列里已有任务全部执行完，再 join 所有工作线程，
 * 并销毁内部的 mutex / cond。返回后线程池不可再用。
 * tp 为 NULL 时安全返回。
 */
void tp_shutdown(thread_pool_t *tp);

/** @return 队列中当前待执行的任务数（加锁读取）；tp 为 NULL 时返回 0。 */
size_t tp_pending(thread_pool_t *tp);

/** @return 工作线程数；tp 为 NULL 时返回 0。 */
size_t tp_worker_count(const thread_pool_t *tp);

/** @return 任务队列容量；tp 为 NULL 时返回 0。 */
size_t tp_capacity(const thread_pool_t *tp);

#ifdef __cplusplus
}
#endif

#endif /* LAB07_THREAD_POOL_H */
