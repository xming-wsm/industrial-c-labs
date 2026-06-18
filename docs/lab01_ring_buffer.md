# Lab 1：环形缓冲区（Ring Buffer）

> 所属阶段：Track A - Linux C 与数据结构
> 预计用时：2~4 小时
> 前置：会用 `gcc`、了解指针与数组、读得懂 `struct`

---

## 1. 工业背景：为什么第一关就是它

注塑机控制系统里，环形缓冲区（又叫循环队列 / 循环 FIFO）几乎无处不在：

- **串口 / RS485 收发缓冲**：HMI 触摸屏、上位机、温控模块的数据是一个字节一个字节来的，中断/DMA 把它们丢进缓冲区，主循环再慢慢取走。
- **ADC 采样流**：压力、螺杆位置、温度等被周期性采样，先入缓冲区再处理（滤波、显示、记录）。
- **报警 / 事件日志**：只保留“最近 N 条”，新事件覆盖最旧的。

它的价值在于：**用一块固定大小的内存**，配合“写游标 head”和“读游标 tail”，实现 O(1) 入队/出队，并且**运行时不需要 malloc/free**——这正是嵌入式实时系统最看重的“内存可控、耗时可预测”。

后续的 Lab 6（生产者-消费者）和 Lab 13（STM32 的 UART + DMA）都会直接复用这一关的代码。所以请认真对待它的接口设计。

---

## 2. 学习目标

完成本关后，你应该能：

1. 解释环形缓冲区的 head/tail 游标与“环绕（wrap-around）”原理；
2. 用“显式 count 计数”法正确判定满 / 空；
3. 写出对 NULL 参数安全、不依赖动态内存的工业级 C 接口；
4. 用 xmake 跑通一套单元测试，理解“实现 → 测试 → 过关”的开发闭环。

---

## 3. 核心原理

把一段线性内存想象成首尾相接的“环”：

```
 容量 capacity = 8
 索引:  0   1   2   3   4   5   6   7
       [ ] [ ] [a] [b] [c] [ ] [ ] [ ]
                tail ----------> head
```

- `head`：下一个**写入**的位置；写入后 `head = (head + 1) % capacity`。
- `tail`：下一个**读取**的位置；读取后 `tail = (tail + 1) % capacity`。
- 当 `head` 自增越过末尾，就回绕到 0，这就是“环绕”。

**关键难点：如何区分“空”和“满”？** 因为空和满时都可能有 `head == tail`。常见两种方案：


| 方案                     | 判空             | 判满                     | 代价         |
| ---------------------- | -------------- | ---------------------- | ---------- |
| 留一个空槽                  | `head == tail` | `(head+1)%cap == tail` | 浪费 1 个槽位   |
| **显式 count（本 lab 采用）** | `count == 0`   | `count == capacity`    | 多一个字段，容量全用 |


本实验采用 **显式 count** 方案：结构体里维护 `count` 字段，逻辑最直观，容量也不浪费。

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**，已写好接口）：[labs/lab01_ring_buffer/include/ring_buffer.h](../labs/lab01_ring_buffer/include/ring_buffer.h)
- 你要实现的源文件：[labs/lab01_ring_buffer/src/ring_buffer.c](../labs/lab01_ring_buffer/src/ring_buffer.c)
- 测试（**不要改**）：[labs/lab01_ring_buffer/test/test_ring_buffer.c](../labs/lab01_ring_buffer/test/test_ring_buffer.c)

需要实现的 API（详见头文件注释）：

```c
rb_status_t rb_init(ring_buffer_t *rb, uint8_t *storage, size_t size);
void        rb_reset(ring_buffer_t *rb);
bool        rb_is_empty(const ring_buffer_t *rb);
bool        rb_is_full(const ring_buffer_t *rb);
size_t      rb_count(const ring_buffer_t *rb);
size_t      rb_capacity(const ring_buffer_t *rb);
rb_status_t rb_push(ring_buffer_t *rb, uint8_t byte);
rb_status_t rb_pop(ring_buffer_t *rb, uint8_t *out);
size_t      rb_write(ring_buffer_t *rb, const uint8_t *data, size_t len);
size_t      rb_read(ring_buffer_t *rb, uint8_t *out, size_t len);
```

**约束（务必遵守）**：

1. 不使用 `malloc/free`，存储区由调用者通过 `rb_init` 传入；
2. 所有函数对 NULL 参数安全（按头文件里每个函数的约定返回错误码 / 默认值，绝不崩溃）；
3. `rb_write` / `rb_read` 用循环复用 `rb_push` / `rb_pop` 即可，遇满 / 遇空就停下并返回“实际处理的字节数”。

---

## 5. 推荐实现步骤

1. 先实现 `rb_init` / `rb_reset` 和四个查询函数（`is_empty` / `is_full` / `count` / `capacity`），跑测试看前几个用例变绿。
2. 实现 `rb_push` / `rb_pop`（注意游标环绕和 count 增减），让 FIFO 顺序、满/空、环绕用例通过。
3. 最后用循环实现 `rb_write` / `rb_read`，让批量用例通过。

游标环绕两种等价写法，任选其一：

```c
rb->head = (rb->head + 1) % rb->capacity;   /* 取模，直观 */
/* 或者避免除法（嵌入式常用）： */
if (++rb->head == rb->capacity) rb->head = 0;
```

---

## 6. 构建与测试

在项目根目录执行：

```bash
xmake lab1          # 编译
xmake lab1 test     # 编译并运行测试
```

也可以直接运行测试可执行文件，看每个用例的明细：

```bash
xmake run test_lab01_ring_buffer
```

**未实现时**应当看到大量 `FAIL`（这是正常的起点）；逐步实现后失败数会减少，最终：

```
==== summary: 12 run, 0 failed ====
```

看到 `==== summary: 12 run, 0 failed ====` 即过关。

---

## 7. 思考题（进入下一关前自检）

1. 为什么工业实时系统里偏爱“调用者提供存储区”而不是内部 `malloc`？（提示：内存碎片、耗时确定性、上电即可用）
2. 在“单生产者单消费者”（比如中断里 push、主循环里 pop）场景下，本实现需要加锁吗？哪个字段是潜在的竞争点？（提示：`count` 的读改写）
3. 如果要做“报警日志”那种**写满后覆盖最旧数据**的环形缓冲，`rb_push` 该怎么改？
4. `rb_write` 跨越缓冲区末尾时是逐字节拷贝的；如果想用两次 `memcpy` 提速，边界该怎么算？

把第 2、3 题的思路记下来——Lab 6（生产者-消费者）和 Lab 13（STM32 UART+DMA）会真正用到。

---

## 8. 过关标准

- `xmake lab1 test` 通过（`0 failed`）；
- 没有使用 `malloc/free`；
- 编译无 `-Wall -Wextra` 告警。

完成后告诉我，我会解锁 **Lab 2：命令队列 / 双端队列**。