# Lab 2：命令队列 / 双端队列（Deque）

> 所属阶段：Track A - Linux C 与数据结构
> 预计用时：3~5 小时
> 前置：完成 Lab 1（环形缓冲区），理解 head 游标 + count 计数与环绕

---

## 1. 工业背景：为什么需要双端队列

注塑机控制器要同时接收来自多个角色的"命令"：

- **HMI 触摸屏**：操作工点"开始注塑""手动开模""急停"；
- **上位机 / MES**：下发配方切换、产量查询、远程启停；
- **内部逻辑**：一个注塑周期结束后自动排入下一动作。

这些命令必须排队、按序执行，否则动作会乱。绝大多数命令是"先到先做"（FIFO，从队尾入、队首出）。但有一类命令——比如**急停**——必须立刻排到队首优先执行。

既能从队尾入、又能从队首入/出的队列，就是**双端队列（deque）**。本关你将用一块固定数组实现它，接口风格和 Lab 1 一脉相承（仍然是"调用者提供存储区、不 malloc、O(1) 操作"）。

后续 Lab 7（线程池任务队列）和 Lab 11（Modbus 请求排队）都会复用这种队列结构。

---

## 2. 学习目标

完成本关后，你应该能：

1. 用环形数组同时支持队首/队尾的入队与出队；
2. 正确处理 `push_front` 时 head 游标"后退并环绕"的边界；
3. 用 `(head + count) % capacity` 和 `(head + count - 1) % capacity` 定位队尾；
4. 把 Lab 1 的环形思想从"字节"推广到"任意固定大小结构体"。

---

## 3. 核心原理

仍然是环形数组，但这次我们只存 `head`（队首下标）和 `count`（元素个数），队尾位置由它们推算：

```
 capacity = 4
 索引:   0      1      2      3
        [ ]    [c1]   [c2]   [ ]
                head ------> 队尾在 (head+count-1)%cap
```

| 操作 | 做什么 | 游标变化 |
| --- | --- | --- |
| `push_back` | 写到 `(head+count)%cap` | `count++` |
| `push_front` | `head` 后退环绕后写入新 head | `head = (head==0)?cap-1:head-1`，`count++` |
| `pop_front` | 读 `head` | `head=(head+1)%cap`，`count--` |
| `pop_back` | 读 `(head+count-1)%cap` | `count--`（head 不变） |

判空 / 判满与 Lab 1 一致：`count == 0` 为空，`count == capacity` 为满。

`push_front` 的环绕是本关最容易写错的地方：当 `head == 0` 时要回绕到 `capacity - 1`，而不是 `-1`（size_t 是无符号数，`0 - 1` 会变成天文数字）。

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**，已写好接口）：[labs/lab02_command_queue/include/command_queue.h](../labs/lab02_command_queue/include/command_queue.h)
- 你要实现的源文件：[labs/lab02_command_queue/src/command_queue.c](../labs/lab02_command_queue/src/command_queue.c)
- 测试（**不要改**）：[labs/lab02_command_queue/test/test_command_queue.c](../labs/lab02_command_queue/test/test_command_queue.c)

需要实现的 API：

```c
cq_status_t cq_init(command_queue_t *cq, cmd_t *storage, size_t capacity);
void        cq_reset(command_queue_t *cq);
bool        cq_is_empty(const command_queue_t *cq);
bool        cq_is_full(const command_queue_t *cq);
size_t      cq_count(const command_queue_t *cq);
size_t      cq_capacity(const command_queue_t *cq);
cq_status_t cq_push_back(command_queue_t *cq, cmd_t cmd);
cq_status_t cq_push_front(command_queue_t *cq, cmd_t cmd);
cq_status_t cq_pop_front(command_queue_t *cq, cmd_t *out);
cq_status_t cq_pop_back(command_queue_t *cq, cmd_t *out);
cq_status_t cq_front(const command_queue_t *cq, cmd_t *out);
cq_status_t cq_back(const command_queue_t *cq, cmd_t *out);
```

**约束（务必遵守）**：

1. 不使用 `malloc/free`，存储区（`cmd_t` 数组）由调用者通过 `cq_init` 传入；
2. 所有函数对 NULL 参数安全；
3. `pop_front` / `pop_back` 的 `out` 可为 NULL，表示丢弃但仍要更新计数。

---

## 5. 推荐实现步骤

1. 先写 `cq_init` / `cq_reset` 和四个查询函数，跑测试看 `test_init_*` 变绿。
2. 写 `cq_push_back` / `cq_pop_front`，让 FIFO 用例通过。
3. 写 `cq_push_front` / `cq_pop_back`，重点测试插队与撤销用例。
4. 写 `cq_front` / `cq_back`（peek），最后跑环绕用例 `test_wrap_mixed`。

`push_front` 的 head 后退写法：

```c
cq->head = (cq->head == 0) ? (cq->capacity - 1) : (cq->head - 1);
```

---

## 6. 构建与测试

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build -R lab02_command_queue --output-on-failure
```

直接看明细：

```bash
./build/labs/lab02_command_queue/test_lab02_command_queue
```

全部实现后应看到 `==== summary: 11 run, 0 failed ====`，CTest 显示 `100% tests passed` 即过关。

---

## 7. 思考题

1. 为什么用 `head + count` 推算队尾，而不是再维护一个 `tail` 字段？两种写法各有什么取舍？
2. `size_t` 是无符号的，`push_front` 里如果写成 `head - 1` 在 `head == 0` 时会发生什么？
3. 如果急停命令应该"清空所有待执行命令再插入"，接口该怎么扩展？
4. 这个 deque 现在是单线程安全的。如果多个线程同时 `push_back`，会出什么问题？（提示：Lab 6 会给出加锁版本）

---

## 8. 过关标准

- `ctest` 全绿（`100% tests passed`）；
- 没有使用 `malloc/free`；
- 编译无 `-Wall -Wextra` 告警。

完成后告诉我，我会解锁 **Lab 3：链表 + 静态内存池**。
