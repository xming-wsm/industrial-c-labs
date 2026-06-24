# Lab 8：周期任务调度 / 软件定时器

> 所属阶段：Track B - 并发与系统编程
> 预计用时：3~5 小时
> 前置：完成 Lab 2（队列）；理解函数指针与 tick/中断的概念

---

## 1. 工业背景：不同周期的任务怎么统一管理

注塑机控制器里很多事情是"按固定周期"做的，而且周期各不相同：

| 任务 | 周期 |
| --- | --- |
| 控制环计算 | 1 ms |
| 压力 / 位置采样 | 10 ms |
| 屏幕刷新 | 100 ms |
| 状态上报 | 1 s |

如果给每个任务都开一个硬件定时器或一个线程，资源很快不够用。正确做法是用**一个**周期性的"心跳"（比如硬件 1ms 定时器中断），驱动一个**软件调度器 / 软件定时器**，由它在合适的 tick 触发各个任务的回调。

为了让逻辑**可测试、可复现**，本关用**逻辑时钟（tick 计数）**而不是真实墙钟：外部每调用一次 `sch_tick`，逻辑时钟 +1，调度器据此判断哪些任务到期。测试里我们手动推进 tick，结果完全确定。

---

## 2. 学习目标

完成本关后，你应该能：

1. 用"周期 + 下次到期时刻 next_due"表达一个周期任务；
2. 实现 tick 驱动的到期判断与重新排期（`next_due += period`）；
3. 区分"周期任务"与"一次性任务"（period==0 触发后自动失效）；
4. 用固定任务槽数组实现注册 / 注销 / 复用空槽（不 malloc）。

---

## 3. 核心原理

每个任务记录它的"下次到期绝对时刻" `next_due`。每个 tick：

```c
sch->now++;
for (每个 active 任务 t) {
    if (sch->now >= t->next_due) {
        t->fn(t->arg);                  /* 触发 */
        if (t->period == 0) t->active = false;   /* 一次性 */
        else                t->next_due += t->period;  /* 周期：排下一次 */
    }
}
```

举例：周期 2、首发在 tick 2 的任务，`next_due` 依次为 2 → 4 → 6 …，于是在第 2、4、6… 个 tick 触发。

注册任务时：`next_due = now + first_delay`。`first_delay` 让你能错开各任务的相位（比如让采样和上报不要总在同一个 tick 扎堆）。

任务槽用一个固定数组，`active` 标记是否在用。注册时找第一个空闲槽，注销时把 `active` 置 false（槽可被后续注册复用）。槽下标就是任务 id。

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**）：[labs/lab08_scheduler/include/scheduler.h](../labs/lab08_scheduler/include/scheduler.h)
- 你要实现的源文件：[labs/lab08_scheduler/src/scheduler.c](../labs/lab08_scheduler/src/scheduler.c)
- 测试（**不要改**）：[labs/lab08_scheduler/test/test_scheduler.c](../labs/lab08_scheduler/test/test_scheduler.c)

需要实现的 API：

```c
sch_status_t sch_init(scheduler_t *sch, sch_task_t *storage, size_t capacity);
uint32_t     sch_now(const scheduler_t *sch);
size_t       sch_active_count(const scheduler_t *sch);
size_t       sch_capacity(const scheduler_t *sch);
void         sch_reset(scheduler_t *sch);                  /* 清空任务并归零时钟 */
bool         sch_is_active(const scheduler_t *sch, int id);/* 槽是否在用 */
int          sch_add(scheduler_t *sch, sch_task_fn fn, void *arg,
                     uint32_t period, uint32_t first_delay);
sch_status_t sch_remove(scheduler_t *sch, int id);
size_t       sch_tick(scheduler_t *sch);   /* 返回本 tick 触发的任务数 */
size_t       sch_advance(scheduler_t *sch, uint32_t ticks); /* 连续推进多个 tick */
```

**约束**：

1. 不使用 `malloc/free`，任务槽数组由 `sch_init` 传入；
2. 所有函数对 NULL 参数安全；
3. `period==0` 表示一次性任务，触发后自动失效；
4. `sch_tick` 返回本次触发的任务数（多个任务可在同一 tick 一起到期）；
5. `sch_advance(sch, n)` 等价于连续调用 `sch_tick` n 次（`n==0` 不推进、不触发）；`sch_reset` 清空所有任务并把时钟归零。

---

## 5. 推荐实现步骤

1. 写 `sch_init` / `sch_now` / `sch_active_count`。
2. 写 `sch_add`（找空槽 + 算 next_due）/ `sch_remove`。
3. 写 `sch_tick`（now++ + 遍历触发 + 重新排期），跑周期与一次性用例。

---

## 6. 构建与测试

```bash
xmake lab8          # 编译
xmake lab8 test     # 编译并运行测试
```

直接看明细：

```bash
xmake run test_lab08_scheduler
```

全部实现后应看到 `==== summary: 9 run, 0 failed ====`。

---

## 7. 思考题

1. 现在 `sch_tick` 每次都遍历所有任务（O(n)）。如果有上千个任务，如何用**最小堆 / 时间轮**把"找最近到期任务"降到更优？
2. 用 `next_due += period`（而不是 `next_due = now + period`）有什么好处？哪种能避免周期漂移？
3. 如果某个任务回调耗时超过了一个 tick，会发生什么？这种"超时任务"该如何检测和处理？
4. `next_due` 是 `uint32_t`，逻辑时钟会溢出回绕。`now >= next_due` 的比较在回绕时会出错吗？工业代码常用什么技巧规避？（提示：比较 `(int32_t)(now - next_due) >= 0`）

---

## 8. 过关标准

- `xmake lab8 test` 通过（`0 failed`）；
- 没有使用 `malloc/free`；
- 编译无 `-Wall -Wextra` 告警。

完成后告诉我，我会解锁 **Lab 9：TCP/UDP socket 上位机通信**（进入 Track C 网络通信）。
