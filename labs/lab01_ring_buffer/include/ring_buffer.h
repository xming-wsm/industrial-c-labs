/**
 * @file ring_buffer.h
 * @brief Lab 1 - 环形缓冲区（Ring Buffer / Circular FIFO）API 声明。
 *
 * 工业背景
 * --------
 * 注塑机控制系统里到处都是“字节流缓冲”的需求：
 *   - 串口 / RS485 收发缓冲（HMI、上位机指令、温控模块）；
 *   - ADC 采样流（压力、位置、温度的连续采样）；
 *   - 报警 / 事件日志（最近 N 条循环覆盖）。
 * 环形缓冲区用一块“固定大小”的内存，配合读写两个游标，实现 O(1) 的
 * 入队 / 出队，且不需要在运行时频繁申请/释放内存。
 *
 * 设计约束（请务必遵守，它们都是为后续 STM32 复用做铺垫）
 * ----------------------------------------------------------
 *   1. 不使用 malloc/free：存储区由调用者提供（栈数组 / 全局数组 / DMA 区均可）。
 *   2. 函数对 NULL 参数要安全：返回错误码而不是崩溃。
 *   3. 单生产者 / 单消费者场景下应可做到无锁（本 lab 不要求加锁，
 *      但实现时不要引入会破坏该性质的全局状态）。
 *
 * 容量约定
 * --------
 * 本实现采用“显式 count 计数”方案：结构体里保存 count，
 * 因此 capacity 个槽位可以全部使用（不像“留一个空槽”方案那样浪费 1 字节）。
 */
#ifndef LAB01_RING_BUFFER_H
#define LAB01_RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 操作返回状态码。 */
typedef enum {
    RB_OK = 0,      /**< 成功                         */
    RB_ERR_NULL,    /**< 传入了 NULL 指针             */
    RB_ERR_FULL,    /**< 缓冲区已满，无法写入         */
    RB_ERR_EMPTY,   /**< 缓冲区为空，无法读取         */
    RB_ERR_SIZE     /**< 容量非法（如 size == 0）     */
} rb_status_t;

/**
 * 环形缓冲区控制块。
 * 字段对调用者公开仅为方便测试与教学，正常使用请只通过下方 API 访问。
 */
typedef struct {
    uint8_t *buffer;    /**< 调用者提供的存储区起始地址 */
    size_t   capacity;  /**< 存储区可容纳的字节数       */
    size_t   head;      /**< 写游标：下一个写入位置     */
    size_t   tail;      /**< 读游标：下一个读取位置     */
    size_t   count;     /**< 当前已存放的字节数         */
} ring_buffer_t;

/**
 * 初始化环形缓冲区，绑定调用者提供的存储区。
 *
 * @param rb       控制块指针。
 * @param storage  存储区起始地址（至少 size 字节）。
 * @param size     存储区容量（字节数），必须 > 0。
 * @return RB_OK 成功；RB_ERR_NULL（rb/storage 为 NULL）；RB_ERR_SIZE（size==0）。
 */
rb_status_t rb_init(ring_buffer_t *rb, uint8_t *storage, size_t size);

/**
 * 清空缓冲区（丢弃所有数据），但不改变其绑定的存储区。
 * rb 为 NULL 时安全返回（什么也不做）。
 */
void rb_reset(ring_buffer_t *rb);

/** @return 缓冲区是否为空；rb 为 NULL 时按“空”处理返回 true。 */
bool rb_is_empty(const ring_buffer_t *rb);

/** @return 缓冲区是否已满；rb 为 NULL 时返回 false。 */
bool rb_is_full(const ring_buffer_t *rb);

/** @return 当前已存放的字节数；rb 为 NULL 时返回 0。 */
size_t rb_count(const ring_buffer_t *rb);

/** @return 总容量（字节）；rb 为 NULL 时返回 0。 */
size_t rb_capacity(const ring_buffer_t *rb);

/**
 * 写入单个字节。
 * @return RB_OK 成功；RB_ERR_NULL（rb 为 NULL）；RB_ERR_FULL（已满）。
 */
rb_status_t rb_push(ring_buffer_t *rb, uint8_t byte);

/**
 * 读取单个字节。
 * @param out 读出的字节写入 *out（out 可为 NULL 表示“丢弃该字节”）。
 * @return RB_OK 成功；RB_ERR_NULL（rb 为 NULL）；RB_ERR_EMPTY（为空）。
 */
rb_status_t rb_pop(ring_buffer_t *rb, uint8_t *out);

/**
 * 批量写入：尽可能多地写入，遇满即停。
 * @return 实际写入的字节数（0 ~ len）。rb/data 为 NULL 时返回 0。
 */
size_t rb_write(ring_buffer_t *rb, const uint8_t *data, size_t len);

/**
 * 批量读取：尽可能多地读出，遇空即停。
 * @return 实际读出的字节数（0 ~ len）。rb/out 为 NULL 时返回 0。
 */
size_t rb_read(ring_buffer_t *rb, uint8_t *out, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* LAB01_RING_BUFFER_H */
