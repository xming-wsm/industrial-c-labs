# Lab 11：Modbus TCP 从站模拟

> 所属阶段：Track C - 网络通信与工业协议
> 预计用时：5~7 小时
> 前置：完成 Lab 4（哈希表/寄存器映射思想）、Lab 9（TCP 收发）、Lab 10（帧/字节序）

---

## 1. 工业背景：用标准协议被上位机读写

Modbus 是工业领域最普及的协议之一——注塑机、温控表、伺服驱动、PLC 几乎都支持它。它是**主从（master/slave）**模型：

- **主站（master）**：上位机 / SCADA / 组态软件，主动发请求；
- **从站（slave）**：本注塑机，被动应答。

从站维护一组**保持寄存器（holding registers）**，本质是一片 `uint16_t` 数组。主站通过"寄存器地址"读写注塑机的参数与状态，例如：

```
寄存器 0x0010  注塑压力设定
寄存器 0x0021  当前模具温度
寄存器 0x0030  运行状态
```

（这正好接得上 Lab 4 的"参数名 → 寄存器地址"映射。）

本关实现一个**纯函数式**的 Modbus TCP 从站请求处理器：输入一帧请求、输出一帧应答，不碰网络。把它接到 Lab 9 的 TCP 收发循环上，就是一个能被真实上位机（如 QModMaster）读写的 Modbus TCP 从站。

---

## 2. 学习目标

完成本关后，你应该能：

1. 读懂并解析 Modbus TCP 的 MBAP 头 + PDU 结构；
2. 实现两个最常用功能码：0x03 读保持寄存器、0x06 写单个寄存器；
3. 正确处理大端字节序（寄存器/地址/数量都是高字节在前）；
4. 按规范返回**异常应答**（非法功能码 / 地址 / 数据值）；
5. 体会"协议处理与传输分离"的工程价值（纯函数 = 易测）。

---

## 3. 核心原理

**Modbus TCP ADU = MBAP 头(7B) + PDU**：

```
偏移:  0   1   2   3   4   5   6   7   8  ...
      [事务ID][协议ID=0][ 长度 ][单元][功能码][数据...]
       hi  lo  hi  lo  hi  lo   ID
                          \____ 长度 = 单元ID(1) + PDU 字节数
```

**FC 0x03 读保持寄存器**

```
请求 PDU: [0x03][起始地址 hi,lo][数量 hi,lo]
应答 PDU: [0x03][字节数=2*数量][寄存器值 hi,lo]...
```

校验：数量须在 1..125，否则异常 0x03；起始地址 + 数量 须 <= 寄存器总数，否则异常 0x02。

**FC 0x06 写单个寄存器**

```
请求 PDU: [0x06][地址 hi,lo][值 hi,lo]
应答 PDU: 原样回显请求 PDU
```

校验：地址 >= 寄存器总数则异常 0x02。

**异常应答**：功能码最高位置 1（`FC | 0x80`），后跟一个异常码：

```
[FC|0x80][异常码]    例如读越界 -> [0x83][0x02]
```

大端读写小工具：

```c
uint16_t rd16(const uint8_t *p) { return (p[0] << 8) | p[1]; }
void     wr16(uint8_t *p, uint16_t v) { p[0] = v >> 8; p[1] = v & 0xFF; }
```

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**）：[labs/lab11_modbus_tcp/include/modbus_slave.h](../labs/lab11_modbus_tcp/include/modbus_slave.h)
- 你要实现的源文件：[labs/lab11_modbus_tcp/src/modbus_slave.c](../labs/lab11_modbus_tcp/src/modbus_slave.c)
- 测试（**不要改**）：[labs/lab11_modbus_tcp/test/test_modbus_slave.c](../labs/lab11_modbus_tcp/test/test_modbus_slave.c)

需要实现的 API：

```c
int mb_init(modbus_slave_t *mb, uint16_t *regs, size_t nregs);
int mb_set_register(modbus_slave_t *mb, uint16_t addr, uint16_t value);
int mb_get_register(const modbus_slave_t *mb, uint16_t addr, uint16_t *out);
int mb_process(modbus_slave_t *mb, const uint8_t *req, size_t req_len,
               uint8_t *resp, size_t resp_cap);
```

**约束**：

1. 寄存器表由 `mb_init` 传入（不 malloc）；
2. 所有多字节字段按大端处理；
3. 应答 MBAP 要回显事务ID/单元ID、协议ID 置 0、长度字段正确；
4. 非法功能码/地址/数据值返回**异常应答**（这是正常返回，长度 > 0）；
5. 报文太短 / 协议ID 非 0 / 应答缓冲不足时返回 -1（不产生应答）。

---

## 5. 推荐实现步骤

1. 写 `mb_init` / `mb_set_register` / `mb_get_register`，跑 `test_init_null`。
2. 写大端 `rd16/wr16` 小工具（static）。
3. 写 `mb_process` 的 MBAP 解析 + 应答头回填框架。
4. 实现 FC 0x03，跑 `test_read_holding`。
5. 实现 FC 0x06，跑 `test_write_single`。
6. 补齐异常分支（地址/数值/功能码），跑剩余用例。

---

## 6. 构建与测试

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build -R lab11_modbus_tcp --output-on-failure
```

直接看明细：

```bash
./build/labs/lab11_modbus_tcp/test_lab11_modbus_tcp
```

全部实现后应看到 `==== summary: 8 run, 0 failed ====`。

> 进阶（选做）：把本关的 `mb_process` 接到 Lab 9 的 `tcp_listen/accept/recv_all/send_all` 上，就能用开源工具（如 QModMaster、modpoll）真实地读写你的"注塑机"。

---

## 7. 思考题

1. Modbus 寄存器地址在协议里从 0 开始，但工程文档常写成 4xxxx（如 40001）。这套"4 万偏移"是怎么回事？
2. Modbus TCP 已经跑在可靠的 TCP 上，为什么 Modbus RTU（串口版，Lab 15）反而需要 CRC，而 Modbus TCP 不带 CRC？
3. 把 `mb_process` 写成"纯函数（无网络、无全局状态）"对单元测试有什么好处？这种"协议层与传输层分离"还有什么工程价值？
4. 如果要支持 FC 0x10（写多个寄存器），请求/应答 PDU 该长什么样？需要哪些额外校验？

---

## 8. 过关标准

- `ctest` 全绿（`100% tests passed`）；
- 读/写/异常/畸形报文都按规范处理；
- 字节序正确；编译无 `-Wall -Wextra` 告警。

完成后告诉我——你已完成 Track A/B/C 全部 Linux C 实验！接下来就是 **Track D：STM32H7 嵌入式**（需要先搭建交叉编译工具链与开发板环境，会单独给出指引）。
