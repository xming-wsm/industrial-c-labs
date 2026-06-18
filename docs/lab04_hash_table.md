# Lab 4：哈希表（参数名 → 寄存器地址映射）

> 所属阶段：Track A - Linux C 与数据结构
> 预计用时：4~6 小时
> 前置：完成 Lab 3（链表 + 内存池），理解链表删除的前驱修补

---

## 1. 工业背景：名字与地址之间的翻译官

注塑机有成百上千个参数。人（HMI、上位机）习惯用**名字**操作：

```
"inj_pressure"  注塑压力
"mold_temp"     模具温度
"screw_pos"     螺杆位置
```

而底层寄存器 / Modbus 只认**地址**（数字）：

```
"inj_pressure" -> 0x0010
"mold_temp"    -> 0x0021
```

每次读写参数都要做一次"名字 → 地址"的翻译。若用线性数组逐个 strcmp，是 O(n)，参数一多就拖慢扫描周期。**哈希表**把字符串经哈希函数映射到桶下标，平均查找接近 O(1)。

本关的哈希表会在 Lab 11（Modbus 寄存器映射）和 Lab 16（注塑参数表）里直接复用。

---

## 2. 学习目标

完成本关后，你应该能：

1. 实现一个字符串哈希函数（djb2）并理解"取模映射到桶"；
2. 用**链地址法**处理哈希冲突（每个桶是一条链表）；
3. 复用 Lab3 的静态内存池给冲突链分配节点（不 malloc）；
4. 实现 put（插入或更新）、get、remove，并正确处理键比较与拷贝。

---

## 3. 核心原理

```
 nbuckets = 4，桶数组每格是一条冲突链：

 buckets[0] -> NULL
 buckets[1] -> ["mold_temp":0x21] -> ["fan":0x40] -> NULL   (两个键哈希后都落桶1)
 buckets[2] -> ["inj_pressure":0x10] -> NULL
 buckets[3] -> NULL
```

- **哈希函数 djb2**：

```c
uint32_t h = 5381;
for (const char *p = key; *p; ++p)
    h = h * 33 + (unsigned char)(*p);   /* h*33 == (h<<5)+h */
```

- **桶下标** = `ht_hash(key) % nbuckets`。
- **冲突**：不同键落到同一桶，就挂在该桶的链表上（头插法最简单）。
- **put**：先在桶链上找 key，找到就更新 value；找不到就从内存池取节点、拷贝 key、头插。
- **get / remove**：定位桶，再沿链表 strcmp 比较 key。remove 与 Lab3 一样要修补前驱指针，并把节点还回池。

> 键是字符串：必须**拷贝进节点**（定长 `key[HT_KEY_MAX]`），不能只存指针——调用者的字符串可能是临时栈变量。

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**）：[labs/lab04_hash_table/include/hash_table.h](../labs/lab04_hash_table/include/hash_table.h)
- 你要实现的源文件：[labs/lab04_hash_table/src/hash_table.c](../labs/lab04_hash_table/src/hash_table.c)
- 测试（**不要改**）：[labs/lab04_hash_table/test/test_hash_table.c](../labs/lab04_hash_table/test/test_hash_table.c)

需要实现的 API：

```c
uint32_t    ht_hash(const char *key);
ht_status_t ht_init(hash_table_t *ht, ht_entry_t **buckets, size_t nbuckets,
                    ht_entry_t *pool, size_t capacity);
void        ht_reset(hash_table_t *ht);
size_t      ht_count(const hash_table_t *ht);
size_t      ht_capacity(const hash_table_t *ht);
ht_status_t ht_put(hash_table_t *ht, const char *key, uint16_t value);
ht_status_t ht_get(const hash_table_t *ht, const char *key, uint16_t *out_value);
ht_status_t ht_remove(hash_table_t *ht, const char *key);
```

**约束**：

1. 不使用 `malloc/free`：桶数组与节点池都由 `ht_init` 传入；
2. 所有函数对 NULL 参数安全；
3. 键比较用 `strncmp`/`strcmp`，键写入用定长安全拷贝（保证 `'\0'`）；
4. `ht_put` 同键时更新 value，`count` 不变。

---

## 5. 推荐实现步骤

1. 先写 `ht_hash`（djb2），跑 `test_hash_deterministic`。
2. 写 `ht_init`（清桶 + 串空闲链表）、`ht_count`、`ht_capacity`。
3. 写 `ht_put` / `ht_get`，跑 `test_put_get`、`test_update_existing`。
4. 用 `nbuckets=1` 强制冲突，跑 `test_collisions` 验证冲突链。
5. 写 `ht_remove`（前驱修补 + 还节点），跑剩余用例。

---

## 6. 构建与测试

```bash
xmake lab4          # 编译
xmake lab4 test     # 编译并运行测试
```

直接看明细：

```bash
xmake run test_lab04_hash_table
```

全部实现后应看到 `==== summary: 10 run, 0 failed ====`。

---

## 7. 思考题

1. 为什么哈希函数初值取 5381、乘子取 33？换成普通的"字符相加"会有什么问题？
2. 链地址法 vs 开放定址法（线性探测），各自的优缺点是什么？嵌入式里更偏好哪种、为什么？
3. 负载因子（count / nbuckets）变大时查找会退化成什么复杂度？工业固定容量场景如何权衡桶数？
4. 如果两个不同字符串恰好哈希值相同（哈希碰撞），你的 `ht_get` 还能正确区分它们吗？靠的是哪一步？

---

## 8. 过关标准

- `xmake lab4 test` 通过（`0 failed`）；
- 没有使用 `malloc/free`；
- 编译无 `-Wall -Wextra` 告警。

完成后告诉我，我会解锁 **Lab 5：表驱动有限状态机（注塑周期）**。
