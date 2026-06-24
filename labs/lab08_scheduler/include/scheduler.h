/**
 * @file scheduler.h
 * @brief Lab 8 - 周期任务调度 / 软件定时器 API 声明。
 *
 * 工业背景
 * --------
 * 注塑机控制器里很多事情是"按周期"做的：
 *   - 每 1ms 跑一次控制环；
 *   - 每 10ms 采一次压力；
 *   - 每 100ms 刷一次屏幕；
 *   - 每 1s 上报一次状态。
 * 这些任务有不同的周期，必须由一个统一的"调度器 / 软件定时器"来管理，
 * 在合适的时刻触发对应回调，而不是给每个任务都开一个硬件定时器或线程。
 *
 * 设计思路
 * --------
 * 用一个**逻辑时钟（tick 计数）**驱动：外部每来一个 tick（比如硬件 1ms
 * 定时器中断里调用一次 sch_tick），调度器就检查哪些任务"到期"了并执行。
 * 用逻辑时钟而非真实墙钟，好处是**完全确定性**，便于单元测试。
 *
 * 设计约束
 * --------
 *   1. 任务表由调用者提供（不 malloc）。
 *   2. 函数对 NULL 参数安全。
 *   3. 周期为 0 表示"一次性"任务：触发一次后自动失效。
 */
#ifndef LAB08_SCHEDULER_H
#define LAB08_SCHEDULER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 操作返回状态码。 */
typedef enum {
    SCH_OK = 0,         /**< 成功               */
    SCH_ERR_NULL,       /**< 传入了 NULL 指针   */
    SCH_ERR_ARG         /**< 参数非法 / id 无效 */
} sch_status_t;

/** 任务回调：到期时被调用。 */
typedef void (*sch_task_fn)(void *arg);

/** 一个被调度的任务槽。 */
typedef struct {
    sch_task_fn fn;        /**< 回调函数                       */
    void       *arg;       /**< 回调参数                       */
    uint32_t    period;    /**< 周期（tick 数）；0 表示一次性  */
    uint32_t    next_due;  /**< 下次触发的绝对 tick            */
    bool        active;    /**< 该槽是否在用                   */
} sch_task_t;

/**
 * 调度器控制块。
 * 字段公开仅为方便测试与教学，正常使用请只通过 API 访问。
 */
typedef struct {
    sch_task_t *tasks;      /**< 任务槽数组（调用者提供） */
    size_t      capacity;   /**< 任务槽总数               */
    uint32_t    now;        /**< 当前逻辑时钟（tick）     */
} scheduler_t;

/**
 * 初始化调度器，绑定任务槽数组，逻辑时钟清零。
 * @return SCH_OK；sch/storage 为 NULL 或 capacity==0 -> SCH_ERR_NULL。
 */
sch_status_t sch_init(scheduler_t *sch, sch_task_t *storage, size_t capacity);

/** @return 当前逻辑时钟值；sch 为 NULL 时返回 0。 */
uint32_t sch_now(const scheduler_t *sch);

/** @return 当前在用（active）的任务数；sch 为 NULL 时返回 0。 */
size_t sch_active_count(const scheduler_t *sch);

/** @return 任务槽总数；sch 为 NULL 时返回 0。 */
size_t sch_capacity(const scheduler_t *sch);

/**
 * 清空所有任务并把逻辑时钟归零（保留已绑定的任务槽数组）。
 * sch 为 NULL 时安全返回。
 */
void sch_reset(scheduler_t *sch);

/** @return 指定 id 的任务槽是否在用；sch 为 NULL 或 id 越界时返回 false。 */
bool sch_is_active(const scheduler_t *sch, int id);

/**
 * 注册一个周期任务。
 *
 * @param sch          调度器。
 * @param fn           回调（不可为 NULL）。
 * @param arg          回调参数。
 * @param period       周期（tick）；0 表示一次性任务。
 * @param first_delay  距首次触发的延迟（tick）：首次触发的绝对时刻 = now + first_delay。
 * @return 成功返回任务 id（>= 0，即槽下标）；
 *         sch/fn 为 NULL 或无空闲槽时返回 -1。
 */
int sch_add(scheduler_t *sch, sch_task_fn fn, void *arg,
            uint32_t period, uint32_t first_delay);

/**
 * 注销任务（使该槽失效，不再触发）。
 * @return SCH_OK；sch 为 NULL 或 id 越界/已失效 -> SCH_ERR_NULL / SCH_ERR_ARG。
 */
sch_status_t sch_remove(scheduler_t *sch, int id);

/**
 * 推进一个 tick：逻辑时钟 +1，然后触发所有"到期"（now >= next_due）的任务。
 *   - 周期任务：触发后 next_due += period。
 *   - 一次性任务（period==0）：触发后自动失效。
 * @return 本 tick 实际触发的任务数；sch 为 NULL 时返回 0。
 */
size_t sch_tick(scheduler_t *sch);

/**
 * 连续推进 ticks 个 tick（等价于调用 sch_tick() ticks 次）。
 * @return 这段时间内累计触发的任务总数；sch 为 NULL 时返回 0。
 */
size_t sch_advance(scheduler_t *sch, uint32_t ticks);

#ifdef __cplusplus
}
#endif

#endif /* LAB08_SCHEDULER_H */
