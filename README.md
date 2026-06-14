# Industrial C Labs（注塑机控制系统实验课）

一套以**工业注塑机控制系统**为主题的渐进式 C 语言实验课，目标是同时提升两项能力：

1. **Linux 环境下的 C 开发能力**
2. **嵌入式开发能力**（基于 STM32H7 开发板，后期解锁）

做法借鉴开源课程的 lab：每一关都提供 **写好的头文件（接口）** + **待你填充的实现桩** + **完整的测试**。你实现 API、跑通测试后即可解锁下一关。

完整规划见 [docs/ROADMAP.md](docs/ROADMAP.md)。

---

## 目录结构

```text
industrial-c-labs/
├── CMakeLists.txt            # 顶层：project + enable_testing + 逐关解锁
├── README.md                 # 本文件
├── docs/
│   ├── ROADMAP.md            # 全部 lab 路线图（教学大纲）
│   └── lab01_ring_buffer.md  # Lab 1 实验指导书
├── common/
│   └── test_framework.h      # 极简单元测试框架（header-only，零依赖）
└── labs/
    └── lab01_ring_buffer/
        ├── CMakeLists.txt
        ├── include/ring_buffer.h    # API 声明（已写好，只读）
        ├── src/ring_buffer.c        # 实现桩（← 你在这里写）
        └── test/test_ring_buffer.c  # 测试（不要改）
```

---

## 环境要求

- Ubuntu / Linux
- `gcc`、`cmake`（>= 3.16）、`make`

如果还没装：

```bash
sudo apt update
sudo apt install -y build-essential cmake
```

---

## 构建与测试

```bash
# 1. 配置（首次或改了 CMakeLists 后执行，生成 build/ 目录）
cmake -S . -B build

# 2. 编译
cmake --build build

# 3. 跑所有测试
ctest --test-dir build --output-on-failure
```

只跑某一关：

```bash
ctest --test-dir build -R lab01_ring_buffer --output-on-failure
```

直接运行某关测试可执行文件看明细：

```bash
./build/labs/lab01_ring_buffer/test_lab01_ring_buffer
```

---

## 做一关的标准流程

1. 读对应的实验指导书（`docs/labNN_*.md`）。
2. 打开 `labs/labNN_*/src/*.c`，把每个 `TODO` 实现掉。
3. 编译并跑测试，看到 `100% tests passed` 即过关。
4. 反馈过关，解锁下一关。

> 刚拿到项目时，Lab 1 的测试是**全部失败**的——这是正常的起点，因为实现桩还没填。

---

## 当前进度

- **Lab 1：环形缓冲区** ← 从这里开始，见 [docs/lab01_ring_buffer.md](docs/lab01_ring_buffer.md)

Track A/B/C（Lab 1~11）的指导书与脚手架（头文件接口 + 实现桩 + 测试 + CMake）**均已就绪**，按解锁顺序逐关推进即可：

| 关 | 指导书 |
| --- | --- |
| Lab 2 命令队列 / 双端队列 | [docs/lab02_command_queue.md](docs/lab02_command_queue.md) |
| Lab 3 链表 + 静态内存池 | [docs/lab03_linked_list_pool.md](docs/lab03_linked_list_pool.md) |
| Lab 4 哈希表 | [docs/lab04_hash_table.md](docs/lab04_hash_table.md) |
| Lab 5 表驱动状态机 | [docs/lab05_fsm.md](docs/lab05_fsm.md) |
| Lab 6 pthread 生产者-消费者 | [docs/lab06_producer_consumer.md](docs/lab06_producer_consumer.md) |
| Lab 7 线程池 + 任务队列 | [docs/lab07_thread_pool.md](docs/lab07_thread_pool.md) |
| Lab 8 周期任务调度 / 定时器 | [docs/lab08_scheduler.md](docs/lab08_scheduler.md) |
| Lab 9 TCP/UDP socket | [docs/lab09_socket_comm.md](docs/lab09_socket_comm.md) |
| Lab 10 协议帧解析 + CRC | [docs/lab10_frame_parser.md](docs/lab10_frame_parser.md) |
| Lab 11 Modbus TCP 从站 | [docs/lab11_modbus_tcp.md](docs/lab11_modbus_tcp.md) |

> 每关默认在顶层 `CMakeLists.txt` 中处于注释（未解锁）状态，过一关就取消下一行 `add_subdirectory(...)` 注释来解锁下一关，保持"实现 → 测试 → 解锁"的闭环。STM32H7 嵌入式（Lab 12~16）见路线图，会在搭好工具链后单独提供。
