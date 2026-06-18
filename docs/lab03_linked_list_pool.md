# Lab 3：链表 + 静态内存池（报警链表）

> 所属阶段：Track A - Linux C 与数据结构
> 预计用时：3~5 小时
> 前置：熟悉指针、`struct` 自引用，理解"不能 malloc"的实时约束

---

## 1. 工业背景：数量不定的报警怎么存

注塑机运行中报警数量是**动态**的：可能 0 条，也可能瞬间冒出十几条（超温、超压、缺料、伺服故障、安全门未关……）。报警要能随时新增、随时按报警码清除、可遍历。

数组放不下"数量不定"，最自然的结构是**链表**。但嵌入式 / 实时系统里有条铁律：**运行时不要 malloc/free**——它会带来内存碎片、耗时不确定，甚至失败。

工业界的标准答案是 **静态内存池 + 自由链表（free list）**：

- 上电时拿到一块固定的节点数组；
- 把所有空闲节点串成一条"空闲链表"；
- "分配"就是从空闲链表头摘一个（O(1)）；
- "释放"就是把节点还回空闲链表头（O(1)）。

你既获得了链表的灵活，又保住了"内存可控、耗时可预测"。本关实现的报警链表会在 Lab 10（帧解析的对象缓存）里被借鉴。

---

## 2. 学习目标

完成本关后，你应该能：

1. 用自引用 `struct`（`next` 指针）实现单向链表的增、删、查；
2. 理解并实现"自由链表"式的静态内存池（O(1) alloc/free）；
3. 正确处理链表删除时的"前驱指针"修补，避免断链；
4. 解释为什么实时系统用内存池替代 `malloc`。

---

## 3. 核心原理

一个节点数组，既能当"空闲池"又能当"业务链表"，区别只在于它挂在哪条链上：

```
 初始（全空闲）:  free_head -> [0] -> [1] -> [2] -> [3] -> NULL
                  list_head -> NULL

 add 两条报警后:  free_head -> [2] -> [3] -> NULL
                  list_head -> [0:codeA] -> [1:codeB] -> NULL
```

- **alloc**：`node = free_head; free_head = free_head->next; used++`
- **free**：`node->next = free_head; free_head = node; used--`
- **add**：alloc 一个节点，填好 `code/value`，挂到业务链表尾。
- **remove**：沿业务链表找首个匹配 `code` 的节点，**用前驱的 next 跳过它**，再 free 还回池。

删除时的前驱修补是最易错处：

```c
ll_node_t *prev = NULL, *cur = al->list_head;
while (cur) {
    if (cur->code == code) {
        if (prev) prev->next = cur->next;   /* 中间/尾部 */
        else      al->list_head = cur->next; /* 删的是头节点 */
        al_node_free(al, cur);
        return LL_OK;
    }
    prev = cur;
    cur = cur->next;
}
```

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**）：[labs/lab03_linked_list_pool/include/alarm_list.h](../labs/lab03_linked_list_pool/include/alarm_list.h)
- 你要实现的源文件：[labs/lab03_linked_list_pool/src/alarm_list.c](../labs/lab03_linked_list_pool/src/alarm_list.c)
- 测试（**不要改**）：[labs/lab03_linked_list_pool/test/test_alarm_list.c](../labs/lab03_linked_list_pool/test/test_alarm_list.c)

需要实现的 API：

```c
ll_status_t al_init(alarm_list_t *al, ll_node_t *storage, size_t capacity);
void        al_reset(alarm_list_t *al);
size_t      al_count(const alarm_list_t *al);
size_t      al_capacity(const alarm_list_t *al);
bool        al_is_full(const alarm_list_t *al);
ll_node_t  *al_node_alloc(alarm_list_t *al);          /* 内存池底层 */
void        al_node_free(alarm_list_t *al, ll_node_t *node);
ll_status_t al_add(alarm_list_t *al, uint16_t code, int32_t value);
bool        al_find(const alarm_list_t *al, uint16_t code, int32_t *out_value);
ll_status_t al_remove(alarm_list_t *al, uint16_t code);
```

**约束**：

1. 不使用 `malloc/free`，节点数组由 `al_init` 传入；
2. 所有函数对 NULL 参数安全；
3. `al_node_alloc` / `al_node_free` 必须是 O(1)（自由链表，不要遍历）；
4. `al_add` 把新报警追加到**链表尾部**（保持插入顺序）；`al_remove` 只删**第一个**匹配项。

---

## 5. 推荐实现步骤

1. 写 `al_init`：用 for 循环把 `pool[i].next = &pool[i+1]`，末节点 `next=NULL`，`free_head=&pool[0]`。
2. 写 `al_node_alloc` / `al_node_free`，跑 `test_pool_alloc_free`。
3. 写 `al_add` / `al_find`，跑 `test_add_find`。
4. 写 `al_remove`（注意前驱修补），跑移除相关用例。
5. `al_reset` 可直接复用 init 的串链逻辑。

---

## 6. 构建与测试

```bash
xmake lab3          # 编译
xmake lab3 test     # 编译并运行测试
```

直接看明细：

```bash
xmake run test_lab03_linked_list_pool
```

全部实现后应看到 `==== summary: 10 run, 0 failed ====`。

---

## 7. 思考题

1. 为什么 `alloc`/`free` 都从空闲链表的**头部**操作？换成尾部会怎样？
2. 如果想支持"按报警码去重"（同一个 code 只存一条），`al_add` 该怎么改？
3. `al_remove` 删除中间节点为什么必须记住前驱？不记前驱有没有别的删法？（提示：单链表删除的"覆盖后继"技巧）
4. 池里节点被 free 后，它的 `code/value` 还在内存里。这会带来什么安全隐患，工业代码通常如何处理？

---

## 8. 过关标准

- `xmake lab3 test` 通过（`0 failed`）；
- 没有使用 `malloc/free`；
- `alloc` / `free` 为 O(1)；
- 编译无 `-Wall -Wextra` 告警。

完成后告诉我，我会解锁 **Lab 4：哈希表（参数名 → 寄存器地址映射）**。
