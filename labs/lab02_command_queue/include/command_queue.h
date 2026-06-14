/**
 * @file command_queue.h
 * @brief Lab 2 - 命令队列 / 双端队列（Deque）API 声明。
 *
 * 工业背景
 * --------
 * 注塑机控制器要同时面对多个"下命令"的来源：
 *   - HMI 触摸屏：操作工点"开始注塑""急停""手动开模"；
 *   - 上位机 / MES：下发配方切换、产量查询；
 *   - 内部逻辑：周期结束自动排入下一动作。
 * 这些命令需要排队、按序执行，偶尔还要"插队"（比如急停必须排到队首）。
 * 这正是一个**双端队列（deque）**：常规命令从队尾入、队首出（FIFO），
 * 高优先级命令可以从队首插入。
 *
 * 设计约束（与 Lab1 一脉相承）
 * ----------------------------
 *   1. 不使用 malloc/free：存储区（cmd_t 数组）由调用者提供。
 *   2. 函数对 NULL 参数安全：返回错误码而不是崩溃。
 *   3. O(1) 入队 / 出队：用环形数组 + head 游标 + count 计数实现，
 *      与 Lab1 的环形缓冲思路一致，只是元素从 uint8_t 变成 cmd_t。
 */
#ifndef LAB02_COMMAND_QUEUE_H
#define LAB02_COMMAND_QUEUE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 一条命令：id 标识动作，arg 携带参数（如目标位置、配方号）。 */
typedef struct {
    uint16_t id;    /**< 命令编号 */
    int32_t  arg;   /**< 命令参数 */
} cmd_t;

/** 操作返回状态码。 */
typedef enum {
    CQ_OK = 0,      /**< 成功                     */
    CQ_ERR_NULL,    /**< 传入了 NULL 指针         */
    CQ_ERR_FULL,    /**< 队列已满                 */
    CQ_ERR_EMPTY,   /**< 队列为空                 */
    CQ_ERR_SIZE     /**< 容量非法（capacity==0） */
} cq_status_t;

/**
 * 双端队列控制块。
 * 字段公开仅为方便测试与教学，正常使用请只通过下方 API 访问。
 *
 * 用环形数组实现：head 指向队首元素；队尾元素在 (head + count - 1) % capacity。
 */
typedef struct {
    cmd_t  *buffer;     /**< 调用者提供的存储区（cmd_t 数组） */
    size_t  capacity;   /**< 可容纳的命令条数                 */
    size_t  head;       /**< 队首元素下标                     */
    size_t  count;      /**< 当前命令条数                     */
} command_queue_t;

/**
 * 初始化队列，绑定调用者提供的存储区。
 *
 * @param cq        控制块指针。
 * @param storage   cmd_t 数组起始地址（至少 capacity 个元素）。
 * @param capacity  可容纳的命令条数，必须 > 0。
 * @return CQ_OK；CQ_ERR_NULL（cq/storage 为 NULL）；CQ_ERR_SIZE（capacity==0）。
 */
cq_status_t cq_init(command_queue_t *cq, cmd_t *storage, size_t capacity);

/** 清空队列（丢弃所有命令），保留绑定的存储区。cq 为 NULL 时安全返回。 */
void cq_reset(command_queue_t *cq);

/** @return 是否为空；cq 为 NULL 时按"空"返回 true。 */
bool cq_is_empty(const command_queue_t *cq);

/** @return 是否已满；cq 为 NULL 时返回 false。 */
bool cq_is_full(const command_queue_t *cq);

/** @return 当前命令条数；cq 为 NULL 时返回 0。 */
size_t cq_count(const command_queue_t *cq);

/** @return 总容量；cq 为 NULL 时返回 0。 */
size_t cq_capacity(const command_queue_t *cq);

/**
 * 从队尾入队（常规路径）。
 * @return CQ_OK；CQ_ERR_NULL（cq 为 NULL）；CQ_ERR_FULL（已满）。
 */
cq_status_t cq_push_back(command_queue_t *cq, cmd_t cmd);

/**
 * 从队首入队（高优先级"插队"，如急停）。
 * @return CQ_OK；CQ_ERR_NULL（cq 为 NULL）；CQ_ERR_FULL（已满）。
 */
cq_status_t cq_push_front(command_queue_t *cq, cmd_t cmd);

/**
 * 从队首出队（最常用：取下一条要执行的命令）。
 * @param out 出队的命令写入 *out（out 可为 NULL 表示丢弃）。
 * @return CQ_OK；CQ_ERR_NULL（cq 为 NULL）；CQ_ERR_EMPTY（为空）。
 */
cq_status_t cq_pop_front(command_queue_t *cq, cmd_t *out);

/**
 * 从队尾出队（撤销最近一条入队的命令）。
 * @param out 出队的命令写入 *out（out 可为 NULL 表示丢弃）。
 * @return CQ_OK；CQ_ERR_NULL（cq 为 NULL）；CQ_ERR_EMPTY（为空）。
 */
cq_status_t cq_pop_back(command_queue_t *cq, cmd_t *out);

/**
 * 查看队首命令但不出队（peek）。
 * @return CQ_OK；CQ_ERR_NULL（cq/out 为 NULL）；CQ_ERR_EMPTY（为空）。
 */
cq_status_t cq_front(const command_queue_t *cq, cmd_t *out);

/**
 * 查看队尾命令但不出队（peek）。
 * @return CQ_OK；CQ_ERR_NULL（cq/out 为 NULL）；CQ_ERR_EMPTY（为空）。
 */
cq_status_t cq_back(const command_queue_t *cq, cmd_t *out);

#ifdef __cplusplus
}
#endif

#endif /* LAB02_COMMAND_QUEUE_H */
