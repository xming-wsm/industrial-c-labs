# Lab 5：表驱动有限状态机（注塑周期）

> 所属阶段：Track A - Linux C 与数据结构
> 预计用时：3~5 小时
> 前置：理解函数指针、`const` 数组、enum

---

## 1. 工业背景：注塑周期就是一台状态机

一个注塑生产周期是严格有序的动作链：

```
空闲 IDLE → 合模 CLAMP → 射胶 INJECT → 保压 HOLD → 储料 CHARGE
        → 冷却 COOL → 开模 OPEN → 顶出 EJECT → (回到 IDLE)
```

每个阶段在它的"完成事件"到来时推进到下一阶段。此外还有横切规则：**任何工作阶段收到急停（ESTOP）都要立刻进入故障态**，故障态只接受"复位"。

如果用一堆 `if (state == ... && event == ...)` 来写，很快就会变成几百行难以维护、容易漏分支的"意大利面条"。工业界的标准解法是**表驱动状态机**：把每条规则写成数据表里的一行：

```
(当前状态, 事件) -> (目标状态, 动作)
```

引擎只做一件事：查表、跳转、（可选）执行动作。**逻辑与数据分离**——加规则就是加一行数据，不动引擎代码。

本关写的引擎会在 Lab 10（用状态机解析协议字节流）和 Lab 16（完整注塑流程）里复用。

---

## 2. 学习目标

完成本关后，你应该能：

1. 用"转移表 + 引擎"的方式表达状态机，理解数据/逻辑分离的价值；
2. 实现一个通用、与具体业务解耦的 `fsm_dispatch`（查表 + 跳转 + 回调）；
3. 用函数指针实现"转移动作"回调，并通过 `ctx` 与业务数据交互；
4. 正确处理"当前状态对该事件无转移"的情况（忽略而非崩溃）。

---

## 3. 核心原理

转移表是一个 `const` 数组，每行是一条规则：

```c
typedef struct {
    int            from;    /* 源状态   */
    int            event;   /* 触发事件 */
    int            to;      /* 目标状态 */
    fsm_action_fn  action;  /* 动作(可NULL) */
} fsm_transition_t;
```

引擎的 `dispatch` 逻辑非常短：

```c
for (size_t i = 0; i < fsm->table_len; i++) {
    if (fsm->table[i].from == fsm->state &&
        fsm->table[i].event == event) {
        fsm->state = fsm->table[i].to;
        if (fsm->table[i].action) fsm->table[i].action(fsm->ctx);
        return FSM_OK;
    }
}
return FSM_NO_TRANSITION;   /* 没有匹配规则：忽略事件，状态不变 */
```

注意引擎**完全不关心** `IDLE`/`INJECT` 这些含义——它只比较整数。状态和事件的语义都写在业务表里。这正是"通用引擎"的精髓。

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**）：[labs/lab05_fsm/include/fsm.h](../labs/lab05_fsm/include/fsm.h)
- 你要实现的源文件：[labs/lab05_fsm/src/fsm.c](../labs/lab05_fsm/src/fsm.c)
- 测试（**不要改**）：[labs/lab05_fsm/test/test_fsm.c](../labs/lab05_fsm/test/test_fsm.c)

需要实现的 API（只有三个函数，重点全在 `fsm_dispatch`）：

```c
fsm_status_t fsm_init(fsm_t *fsm, const fsm_transition_t *table, size_t table_len,
                      int initial, void *ctx);
int          fsm_state(const fsm_t *fsm);
fsm_status_t fsm_dispatch(fsm_t *fsm, int event);
```

测试文件里已经替你定义好了**注塑周期的转移表**（顺序流程 + 任意工作态急停 + 故障复位）。你只需把引擎实现正确，它就能驱动这张表。

**约束**：

1. 不使用 `malloc/free`，转移表由调用者提供（const 静态数组）；
2. 所有函数对 NULL 参数安全（`fsm_state(NULL)` 返回 -1）；
3. `dispatch` 未命中时状态保持不变并返回 `FSM_NO_TRANSITION`；
4. 命中且 `action` 非 NULL 时必须调用 `action(ctx)`。

---

## 5. 推荐实现步骤

1. 写 `fsm_init`（参数校验 + 绑定字段 + 设初始状态）和 `fsm_state`。
2. 写 `fsm_dispatch`（线性扫描转移表）。
3. 跑 `test_full_cycle` 看完整周期是否走通，再跑急停 / 忽略 / 多周期用例。

---

## 6. 构建与测试

```bash
xmake lab5          # 编译
xmake lab5 test     # 编译并运行测试
```

直接看明细：

```bash
xmake run test_lab05_fsm
```

全部实现后应看到 `==== summary: 6 run, 0 failed ====`。

---

## 7. 思考题

1. 线性扫描转移表是 O(行数)。状态/事件很多时如何加速？（提示：二维表 `next[state][event]`，或按 from 状态分组）
2. 本引擎只有"转移动作"。如果还想要"进入某状态时执行 entry 动作、离开时执行 exit 动作"，结构该怎么扩展？
3. 表里同一个 `(from, event)` 出现两行会怎样？引擎该取第一条、还是应当在 init 时校验唯一性？
4. 急停规则现在对每个工作态都写了一行（重复）。有没有办法用"通配 from"减少重复？这样做的代价是什么？

---

## 8. 过关标准

- `xmake lab5 test` 通过（`0 failed`）；
- 没有使用 `malloc/free`；
- 编译无 `-Wall -Wextra` 告警。

完成后告诉我，我会解锁 **Lab 6：pthread 生产者-消费者**（开始进入 Track B 并发编程）。
