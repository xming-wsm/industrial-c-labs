/**
 * @file sync_buffer.h
 * @brief Lab 6 - 线程安全阻塞缓冲区（生产者-消费者）API 声明。
 *
 * 工业背景
 * --------
 * 注塑机软件里典型的"采集线程 → 处理线程"模型：
 *   - 采集线程（生产者）：从串口/ADC 不停拿数据，丢进缓冲区；
 *   - 处理线程（消费者）：从缓冲区取数据做滤波、记录、上报。
 * 两个线程速度不一致，需要一个**线程安全**的缓冲区在中间解耦：
 *   - 缓冲区满时，生产者应**阻塞等待**（而不是丢数据或忙等）；
 *   - 缓冲区空时，消费者应**阻塞等待**；
 *   - 关闭时，要能优雅唤醒所有阻塞线程。
 *
 * 本关在 Lab1 环形缓冲的基础上，加一把互斥锁和两个条件变量，
 * 实现经典的"有界缓冲 / 生产者-消费者"同步原语。
 *
 * 设计约束
 * --------
 *   1. 存储区由调用者提供（不 malloc 数据区）。
 *   2. put/get 用条件变量阻塞，禁止忙等（busy-wait）。
 *   3. 关闭后：put 立即返回 SB_CLOSED；get 先把残留数据取完，再返回 SB_CLOSED。
 */
#ifndef LAB06_SYNC_BUFFER_H
#define LAB06_SYNC_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 操作返回状态码。 */
typedef enum {
    SB_OK = 0,      /**< 成功                         */
    SB_ERR_NULL,    /**< 传入了 NULL 指针             */
    SB_ERR_SIZE,    /**< 容量非法（size==0）         */
    SB_CLOSED       /**< 缓冲区已关闭（且无数据可取） */
} sb_status_t;

/**
 * 线程安全阻塞缓冲区控制块。
 * 内部用环形数组 + 互斥锁 + 两个条件变量（非空 / 非满）实现。
 */
typedef struct {
    uint8_t        *buffer;     /**< 调用者提供的存储区 */
    size_t          capacity;   /**< 容量（字节）       */
    size_t          head;       /**< 写游标             */
    size_t          tail;       /**< 读游标             */
    size_t          count;      /**< 当前字节数         */
    bool            closed;     /**< 是否已关闭         */
    pthread_mutex_t lock;       /**< 保护以上字段       */
    pthread_cond_t  not_empty;  /**< "有数据可取"信号   */
    pthread_cond_t  not_full;   /**< "有空位可写"信号   */
} sync_buffer_t;

/**
 * 初始化缓冲区，绑定存储区，并初始化互斥锁与条件变量。
 *
 * @param sb       控制块。
 * @param storage  存储区起始地址（至少 size 字节）。
 * @param size     容量（字节），必须 > 0。
 * @return SB_OK；SB_ERR_NULL（sb/storage 为 NULL）；SB_ERR_SIZE（size==0）。
 */
sb_status_t sb_init(sync_buffer_t *sb, uint8_t *storage, size_t size);

/**
 * 销毁缓冲区，释放互斥锁与条件变量（不释放存储区）。
 * 调用前应确保没有线程仍阻塞在该缓冲区上（一般先 sb_close 再 join 线程）。
 */
void sb_destroy(sync_buffer_t *sb);

/**
 * 写入一个字节；满时阻塞等待，直到有空位或缓冲区被关闭。
 * @return SB_OK 成功；SB_CLOSED（已关闭，写入被拒绝）；SB_ERR_NULL。
 */
sb_status_t sb_put(sync_buffer_t *sb, uint8_t byte);

/**
 * 读取一个字节；空时阻塞等待，直到有数据或缓冲区被关闭且已排空。
 * @param out 读出的字节写入 *out（out 可为 NULL 表示丢弃）。
 * @return SB_OK 成功；SB_CLOSED（已关闭且无数据）；SB_ERR_NULL。
 */
sb_status_t sb_get(sync_buffer_t *sb, uint8_t *out);

/**
 * 关闭缓冲区：唤醒所有阻塞的生产者/消费者。
 * 关闭后 sb_put 立即返回 SB_CLOSED；sb_get 仍可取走残留数据，取完返回 SB_CLOSED。
 * sb 为 NULL 时安全返回。
 */
void sb_close(sync_buffer_t *sb);

/** @return 当前字节数（加锁读取）；sb 为 NULL 时返回 0。 */
size_t sb_count(sync_buffer_t *sb);

#ifdef __cplusplus
}
#endif

#endif /* LAB06_SYNC_BUFFER_H */
