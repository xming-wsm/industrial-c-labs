# Lab 6：pthread 生产者-消费者（线程安全缓冲区）

> 所属阶段：Track B - 并发与系统编程
> 预计用时：4~6 小时
> 前置：完成 Lab 1（环形缓冲区）；了解线程、竞态、死锁的基本概念

---

## 1. 工业背景：采集线程与处理线程的解耦

注塑机软件几乎都是多线程的，最经典的一对是：

- **采集线程（生产者）**：从串口 / ADC 不停读数据，丢进缓冲区；
- **处理线程（消费者）**：从缓冲区取数据，做滤波、显示、记录、上报。

两者速度天然不一致：采集可能突发很快，处理偶尔被磁盘/网络拖慢。中间需要一个**线程安全的有界缓冲区**来削峰填谷，并满足：

- 缓冲区**满**时，生产者应**阻塞等待**（不丢数据、不忙等烧 CPU）；
- 缓冲区**空**时，消费者应**阻塞等待**；
- 系统停机时，要能**优雅唤醒**所有阻塞线程，让它们干净退出。

这就是操作系统课里的"生产者-消费者 / 有界缓冲"问题。本关在 Lab 1 环形缓冲之上，加一把互斥锁和两个条件变量来实现它。

---

## 2. 学习目标

完成本关后，你应该能：

1. 用 `pthread_mutex_t` 保护共享数据，避免竞态；
2. 用 `pthread_cond_t` 实现"满则等、空则等"的阻塞，而非忙等；
3. 理解并正确处理**虚假唤醒**（为什么必须用 `while` 而不是 `if` 重判条件）；
4. 设计"关闭（close）"语义，优雅唤醒并退出所有阻塞线程。

---

## 3. 核心原理

数据结构 = Lab1 的环形缓冲 + 一把锁 + 两个条件变量：

```
not_full  : "现在有空位可写了" —— 由消费者取走数据后发出
not_empty : "现在有数据可取了" —— 由生产者放入数据后发出
```

`sb_put` 的骨架：

```c
pthread_mutex_lock(&sb->lock);
while (sb->count == sb->capacity && !sb->closed)
    pthread_cond_wait(&sb->not_full, &sb->lock);   /* 满则等 */
if (sb->closed) { pthread_mutex_unlock(&sb->lock); return SB_CLOSED; }
/* 写入一个字节，count++ */
pthread_cond_signal(&sb->not_empty);               /* 唤醒消费者 */
pthread_mutex_unlock(&sb->lock);
return SB_OK;
```

`sb_get` 对称，但关闭语义不同：**关闭后仍要把残留数据取完**，取空了才返回 `SB_CLOSED`：

```c
while (sb->count == 0 && !sb->closed)
    pthread_cond_wait(&sb->not_empty, &sb->lock);
if (sb->count == 0 /* 且已 closed */) { unlock; return SB_CLOSED; }
```

**为什么用 `while` 而不是 `if`？** `pthread_cond_wait` 可能"虚假唤醒"，也可能被 `broadcast` 唤醒后发现条件其实不满足（别的线程抢先消费了）。所以醒来后必须**重新检查**条件——这是并发编程的铁律。

`sb_close` 设置 `closed=true` 后要 `pthread_cond_broadcast` 唤醒**所有**阻塞者，让它们重新判断条件并退出。

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**）：[labs/lab06_producer_consumer/include/sync_buffer.h](../labs/lab06_producer_consumer/include/sync_buffer.h)
- 你要实现的源文件：[labs/lab06_producer_consumer/src/sync_buffer.c](../labs/lab06_producer_consumer/src/sync_buffer.c)
- 测试（**不要改**）：[labs/lab06_producer_consumer/test/test_sync_buffer.c](../labs/lab06_producer_consumer/test/test_sync_buffer.c)

需要实现的 API：

```c
sb_status_t sb_init(sync_buffer_t *sb, uint8_t *storage, size_t size);
void        sb_destroy(sync_buffer_t *sb);
sb_status_t sb_put(sync_buffer_t *sb, uint8_t byte);   /* 满则阻塞 */
sb_status_t sb_get(sync_buffer_t *sb, uint8_t *out);   /* 空则阻塞 */
sb_status_t sb_try_put(sync_buffer_t *sb, uint8_t byte);/* 满则立即返回 SB_FULL */
sb_status_t sb_try_get(sync_buffer_t *sb, uint8_t *out);/* 空则立即返回 SB_EMPTY */
void        sb_close(sync_buffer_t *sb);
size_t      sb_count(sync_buffer_t *sb);
size_t      sb_capacity(const sync_buffer_t *sb);
```

**约束**：

1. 阻塞必须用条件变量，**禁止忙等**（不要 `while(full){}` 空转）；
2. 醒来后用 `while` 重判条件，正确处理虚假唤醒；
3. 关闭后：`sb_put` 立即返回 `SB_CLOSED`；`sb_get` 取完残留数据后才返回 `SB_CLOSED`；
4. `sb_try_put` / `sb_try_get` **绝不阻塞**：满返回 `SB_FULL`、空返回 `SB_EMPTY`、已关闭返回 `SB_CLOSED`；
5. 对 NULL 参数安全。

测试除单生产者/单消费者外，还有 **3 生产者 + 3 消费者** 的守恒性用例（小缓冲反复阻塞，校验取走总数 == 写入总数），以及非阻塞 try_* 的满/空/关闭路径。

---

## 5. 推荐实现步骤

1. 先写 `sb_init`（含 mutex/cond 初始化）、`sb_destroy`、`sb_count`，跑单线程用例。
2. 写 `sb_put` / `sb_get` 的加锁与条件等待，跑 `test_single_thread_put_get`。
3. 写 `sb_close` 与关闭语义，跑 `test_close_then_get_drains`。
4. 最后跑多线程用例 `test_spsc_blocking`。

> 调试并发问题时，`helgrind`（`valgrind --tool=helgrind ./test_...`）能帮你发现数据竞争与锁误用。

---

## 6. 构建与测试

```bash
xmake lab6          # 编译
xmake lab6 test     # 编译并运行测试
```

直接看明细：

```bash
xmake run test_lab06_producer_consumer
```

全部实现后应看到 `==== summary: 7 run, 0 failed ====`。

> 若测试**卡住不返回**，多半是漏了某处 `signal`/`broadcast`，或用了 `if` 导致线程错过唤醒后永久阻塞——重点检查 close 路径。

---

## 7. 思考题

1. 如果把 `while` 改成 `if`，在什么时序下会出错？请描述一个具体的崩溃/卡死场景。
2. 为什么 `sb_count` 也要加锁？不加锁读 `count` 有什么风险？
3. `signal`（唤醒一个）和 `broadcast`（唤醒全部）各自适用什么场景？close 为什么必须用 broadcast？
4. 单生产者-单消费者（SPSC）其实可以做成**无锁**的（回顾 Lab1 思考题）。无锁方案相比本关的加锁方案，优势和限制分别是什么？

---

## 8. 过关标准

- `xmake lab6 test` 通过（`0 failed`），且测试能在 1~2 秒内结束（不卡死）；
- 阻塞用条件变量实现，无忙等；
- 编译无 `-Wall -Wextra` 告警。

完成后告诉我，我会解锁 **Lab 7：线程池 + 任务队列**。
