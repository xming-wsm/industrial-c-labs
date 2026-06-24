/**
 * @file socket_comm.h
 * @brief Lab 9 - TCP/UDP socket 上位机通信 API 声明。
 *
 * 工业背景
 * --------
 * 注塑机要和上位机 / MES / 组态软件通信：上报产量与状态、接收配方下发与
 * 远程启停。最常见的承载就是 TCP（可靠、面向连接，适合命令/应答）与
 * UDP（无连接、低开销，适合周期性广播状态）。
 *
 * 本关把 BSD socket 那套繁琐的 API（socket/bind/listen/accept/connect/
 * send/recv）封装成几个"好用、对错误友好"的函数。重点掌握两个易错点：
 *   - send 可能"部分发送"，recv 可能"部分接收" —— 必须循环直到收发够字节；
 *   - 绑定端口 0 让内核分配空闲端口，再用 getsockname 读回真实端口
 *     （测试用它避免端口冲突）。
 *
 * 约定
 * ----
 *   - 所有"返回 fd / 端口"的函数失败时返回 -1 / 0。
 *   - 仅使用 IPv4 + 127.0.0.1 回环即可通过测试。
 */
#ifndef LAB09_SOCKET_COMM_H
#define LAB09_SOCKET_COMM_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- TCP ---- */

/**
 * 创建一个监听 socket，绑定到 127.0.0.1:port 并开始监听。
 * @param port    端口；传 0 表示让内核分配一个空闲端口。
 * @param backlog listen 的 backlog。
 * @return 监听 fd；出错返回 -1。
 */
int tcp_listen(uint16_t port, int backlog);

/**
 * 在监听 fd 上接受一个连接。
 * @return 已连接 fd；出错返回 -1。
 */
int tcp_accept(int listen_fd);

/**
 * 主动连接到 host:port（host 形如 "127.0.0.1"）。
 * @return 已连接 fd；出错返回 -1。
 */
int tcp_connect(const char *host, uint16_t port);

/* ---- 通用收发 / 端口 / 关闭 ---- */

/**
 * 读取 fd 绑定的本地端口（getsockname）。
 * @return 端口号（主机字节序）；出错返回 0。
 */
uint16_t sock_local_port(int fd);

/**
 * 读取已连接 fd 的对端端口（getpeername）。
 * @return 对端端口号（主机字节序）；出错或未连接返回 0。
 */
uint16_t sock_peer_port(int fd);

/**
 * 发送恰好 len 字节（处理部分发送，循环直到发完）。
 * @return 实际发送的字节数（成功时等于 len）；出错返回 -1。
 */
ssize_t sock_send_all(int fd, const void *buf, size_t len);

/**
 * 接收恰好 len 字节（处理部分接收，循环直到收满）。
 * @return 实际接收的字节数：== len 成功；0 表示对端在收满前关闭；-1 出错。
 */
ssize_t sock_recv_all(int fd, void *buf, size_t len);

/** 关闭一个 fd（对 fd < 0 安全，什么也不做）。 */
void sock_close(int fd);

/* ---- UDP ---- */

/**
 * 创建一个 UDP socket 并绑定到 127.0.0.1:port。
 * @param port 端口；传 0 让内核分配。
 * @return UDP fd；出错返回 -1。
 */
int udp_socket(uint16_t port);

/**
 * 向 host:port 发送一个 UDP 数据报。
 * @return 发送的字节数；出错返回 -1。
 */
ssize_t udp_sendto(int fd, const char *host, uint16_t port,
                   const void *buf, size_t len);

/**
 * 接收一个 UDP 数据报（最多 len 字节）。
 * @return 收到的字节数；出错返回 -1。
 */
ssize_t udp_recvfrom(int fd, void *buf, size_t len);

/**
 * 接收一个 UDP 数据报，并回填发送方地址（用于实现 UDP 应答 / echo）。
 * @param out_host  发送方 IP 字符串写入处（可为 NULL）。
 * @param host_cap  out_host 缓冲区容量（含结尾 '\0'）。
 * @param out_port  发送方端口（主机字节序）写入处（可为 NULL）。
 * @return 收到的字节数；出错返回 -1。
 */
ssize_t udp_recvfrom_from(int fd, void *buf, size_t len,
                          char *out_host, size_t host_cap, uint16_t *out_port);

#ifdef __cplusplus
}
#endif

#endif /* LAB09_SOCKET_COMM_H */
