/**
 * @file scheduler.c
 * @brief Lab 8 - 周期任务调度 / 软件定时器（待你完成）
 *
 * 详见 docs/lab08_scheduler.md；测试：xmake lab8 test
 */
#include "scheduler.h"

sch_status_t sch_init(scheduler_t *sch, sch_task_t *storage, size_t capacity) {
    (void)sch;
    (void)storage;
    (void)capacity;
    return SCH_ERR_NULL;
}

uint32_t sch_now(const scheduler_t *sch) {
    (void)sch;
    return 0;
}

size_t sch_active_count(const scheduler_t *sch) {
    (void)sch;
    return 0;
}

int sch_add(scheduler_t *sch, sch_task_fn fn, void *arg,
            uint32_t period, uint32_t first_delay) {
    (void)sch;
    (void)fn;
    (void)arg;
    (void)period;
    (void)first_delay;
    return -1;
}

sch_status_t sch_remove(scheduler_t *sch, int id) {
    (void)sch;
    (void)id;
    return SCH_ERR_NULL;
}

size_t sch_tick(scheduler_t *sch) {
    (void)sch;
    return 0;
}
