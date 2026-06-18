# Lab 9：TCP/UDP socket 上位机通信

> 所属阶段：Track C - 网络通信与工业协议
> 预计用时：4~6 小时
> 前置：了解 TCP/UDP 区别、IP:端口的概念；会查 man 手册

---

## 1. 工业背景：注塑机如何与上位机对话

注塑机不是孤岛，它要和外部系统通信：

- 向 **MES / 上位机** 上报产量、周期时间、报警；
- 接收**配方下发**、远程启停、参数修改；
- 向组态软件 / 看板**周期广播**实时状态。

承载这些通信的，绝大多数是 **TCP** 和 **UDP**：

| | TCP | UDP |
| --- | --- | --- |
| 连接 | 面向连接、可靠、有序 | 无连接、不保证可达 |
| 适合 | 命令/应答、文件、配方 | 周期性状态广播、低延迟遥测 |
| 代价 | 握手、重传开销 | 需自己处理丢包/乱序 |

Linux 下用的是 BSD socket API，但它有不少"坑"，最典型的是：**`send`/`recv` 不保证一次收发完所有字节**。本关你要把这套 API 封装成稳健好用的函数，重点掌握"循环收发"。

本关的收发函数会在 Lab 10（解析收到的字节流）、Lab 11（Modbus TCP）里复用。

---

## 2. 学习目标

完成本关后，你应该能：

1. 用 `socket/bind/listen/accept/connect` 建立 TCP 连接；
2. 正确实现 `send_all` / `recv_all`，处理"部分收发"这一经典坑；
3. 用 UDP 的 `sendto/recvfrom` 收发数据报；
4. 用"绑定端口 0 + `getsockname`"让内核分配端口（避免端口冲突，利于测试）。

---

## 3. 核心原理

**地址结构**（IPv4）：

```c
struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port   = htons(port);                 /* 主机序 -> 网络序 */
inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
```

**TCP 服务端**：`socket → setsockopt(SO_REUSEADDR) → bind → listen → accept`。
**TCP 客户端**：`socket → connect`。

**为什么必须循环收发？** `send(fd, buf, 1000, 0)` 可能只发出 600 字节就返回 600（内核缓冲满）；`recv` 也可能一次只给你一部分。所以：

```c
ssize_t sock_send_all(int fd, const void *buf, size_t len) {
    const char *p = buf;
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, p + sent, len - sent, 0);
        if (n < 0) return -1;          /* 出错 */
        sent += (size_t)n;
    }
    return (ssize_t)sent;
}
```

`recv` 返回 0 表示**对端已关闭连接**（EOF），要和"出错(-1)"区分开。

**端口 0 技巧**：`bind` 时端口填 0，内核会挑一个空闲端口；之后用 `getsockname` 读回真正的端口号。测试正是用它来避免"端口已被占用"导致的偶发失败。

---

## 4. 你要实现什么

文件位置：

- 头文件（**只读**）：[labs/lab09_socket_comm/include/socket_comm.h](../labs/lab09_socket_comm/include/socket_comm.h)
- 你要实现的源文件：[labs/lab09_socket_comm/src/socket_comm.c](../labs/lab09_socket_comm/src/socket_comm.c)
- 测试（**不要改**）：[labs/lab09_socket_comm/test/test_socket_comm.c](../labs/lab09_socket_comm/test/test_socket_comm.c)

需要实现的 API：

```c
int      tcp_listen(uint16_t port, int backlog);
int      tcp_accept(int listen_fd);
int      tcp_connect(const char *host, uint16_t port);
uint16_t sock_local_port(int fd);
ssize_t  sock_send_all(int fd, const void *buf, size_t len);
ssize_t  sock_recv_all(int fd, void *buf, size_t len);
void     sock_close(int fd);
int      udp_socket(uint16_t port);
ssize_t  udp_sendto(int fd, const char *host, uint16_t port, const void *buf, size_t len);
ssize_t  udp_recvfrom(int fd, void *buf, size_t len);
```

**约束**：

1. `send_all` / `recv_all` 必须循环处理部分收发；
2. 出错统一返回 -1（端口类返回 0）；失败时不要泄漏已打开的 fd；
3. 全部用 IPv4 + `127.0.0.1` 即可（测试只用回环）。

测试会在子线程起一个 echo / drain 服务端，并传输多达 20 万字节来逼出"部分收发"。

---

## 5. 推荐实现步骤

1. 写 `sock_close`、`sock_send_all`、`sock_recv_all`（先把收发循环写对）。
2. 写 `tcp_listen`（含 `SO_REUSEADDR`）、`sock_local_port`、`tcp_accept`、`tcp_connect`，跑 `test_tcp_echo`。
3. 跑 `test_tcp_big_transfer` 验证大数据收发。
4. 写 `udp_socket` / `udp_sendto` / `udp_recvfrom`，跑 UDP 用例。

---

## 6. 构建与测试

```bash
xmake lab9          # 编译
xmake lab9 test     # 编译并运行测试
```

直接看明细：

```bash
xmake run test_lab09_socket_comm
```

全部实现后应看到 `==== summary: 4 run, 0 failed ====`。

> 提示：若进程收到 `SIGPIPE` 而退出，是因为往已关闭的连接写数据。可在 send 时加 `MSG_NOSIGNAL` 标志，或忽略 `SIGPIPE`。本关测试不会触发它，但工业代码要注意。

---

## 7. 思考题

1. `send` 返回的字节数小于请求时说明什么？为什么不能假设"一次发完"？
2. `recv` 返回 0 和返回 -1 分别代表什么？你的 `recv_all` 如何区分处理？
3. TCP 是"字节流"，没有消息边界。如果上位机连续发两条命令，接收端可能一次 `recv` 同时读到两条（粘包）。这个问题该在哪一层解决？（提示：正是下一关 Lab 10）
4. UDP 的 `recvfrom` 一次正好收一个数据报。如果发送方发了 2000 字节但接收 buf 只有 64 字节，会发生什么？

---

## 8. 过关标准

- `xmake lab9 test` 通过（`0 failed`）；
- `send_all` / `recv_all` 正确处理部分收发（大数据用例通过）；
- 无 fd 泄漏；编译无 `-Wall -Wextra` 告警。

完成后告诉我，我会解锁 **Lab 10：协议帧解析 + CRC**。
