/**
 * @file scheduler.c
 * @brief Lab 8 - 周期任务调度 / 软件定时器实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_scheduler.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab08_scheduler.md）：
 *   - sch_add：在 tasks[] 里找第一个 active==false 的空槽；填好字段，
 *       next_due = now + first_delay，active=true，返回槽下标作为 id。
 *   - sch_tick：now++；遍历所有 active 任务，若 now >= next_due 则触发：
 *       调用 fn(arg)；若 period==0 则 active=false（一次性），
 *       否则 next_due += period。返回触发计数。
 *   - 全程不使用 malloc/free。
 */
#include "scheduler.h"

sch_status_t sch_init(scheduler_t *sch, sch_task_t *storage, size_t capacity) {
    /* TODO:
     *   1. sch/storage 为 NULL 或 capacity==0 -> SCH_ERR_NULL
     *   2. 绑定 tasks/capacity；now=0；所有槽 active=false -> SCH_OK
     */
    (void)sch;
    (void)storage;
    (void)capacity;
    return SCH_ERR_NULL; /* 占位：实现前测试应当失败 */
}

uint32_t sch_now(const scheduler_t *sch) {
    /* TODO: NULL 返回 0；否则返回 now。 */
    (void)sch;
    return 0;
}

size_t sch_active_count(const scheduler_t *sch) {
    /* TODO: NULL 返回 0；否则统计 active==true 的槽数。 */
    (void)sch;
    return 0;
}

int sch_add(scheduler_t *sch, sch_task_fn fn, void *arg,
            uint32_t period, uint32_t first_delay) {
    /* TODO:
     *   sch/fn 为 NULL -> -1；
     *   找第一个 active==false 的槽（找不到 -> -1）；
     *   填 fn/arg/period，next_due = now + first_delay，active=true；
     *   返回该槽下标。
     */
    (void)sch;
    (void)fn;
    (void)arg;
    (void)period;
    (void)first_delay;
    return -1;
}

sch_status_t sch_remove(scheduler_t *sch, int id) {
    /* TODO:
     *   sch 为 NULL -> SCH_ERR_NULL；
     *   id < 0 或 id >= capacity 或该槽未 active -> SCH_ERR_ARG；
     *   否则 active=false -> SCH_OK。
     */
    (void)sch;
    (void)id;
    return SCH_ERR_NULL;
}

size_t sch_tick(scheduler_t *sch) {
    /* TODO:
     *   sch 为 NULL -> 0；
     *   now++；遍历所有 active 槽，若 now >= next_due：
     *     fn(arg)；若 period==0 则 active=false，否则 next_due += period；fired++。
     *   返回 fired。
     */
    (void)sch;
    return 0;
}
