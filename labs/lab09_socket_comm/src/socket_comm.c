/**
 * @file socket_comm.c
 * @brief Lab 9 - TCP/UDP socket 封装实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_socket_comm.c 全部通过。
 *
 * 你会用到的头文件与调用（Linux）：
 *   #include <sys/socket.h>   socket / bind / listen / accept / connect / send / recv
 *   #include <netinet/in.h>   struct sockaddr_in, htons/ntohs, INADDR_ANY
 *   #include <arpa/inet.h>    inet_pton / inet_addr
 *   #include <unistd.h>       close
 *   #include <string.h>       memset
 *
 * 实现要点（强烈建议先读 docs/lab09_socket_comm.md）：
 *   - 地址结构：sin_family=AF_INET; sin_port=htons(port);
 *     sin_addr 用 inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr)。
 *   - tcp_listen：socket(AF_INET, SOCK_STREAM, 0) -> setsockopt(SO_REUSEADDR)
 *     -> bind -> listen。绑定 127.0.0.1，port 可为 0。
 *   - sock_local_port：getsockname 后 ntohs(addr.sin_port)。
 *   - sock_send_all / sock_recv_all：循环，处理返回值 < len 的"部分收发"，
 *     recv 返回 0 表示对端关闭。
 *   - udp_socket：socket(..., SOCK_DGRAM, 0) 后 bind。
 *   - 任何系统调用失败都要清理已打开的 fd 并返回 -1 / 0。
 */
#include "socket_comm.h"

int tcp_listen(uint16_t port, int backlog) {
    /* TODO: socket -> setsockopt(SO_REUSEADDR) -> bind(127.0.0.1:port) -> listen。
     *       成功返回监听 fd；任一步失败返回 -1（记得 close 已开的 fd）。 */
    (void)port;
    (void)backlog;
    return -1; /* 占位：实现前测试应当失败 */
}

int tcp_accept(int listen_fd) {
    /* TODO: accept(listen_fd, NULL, NULL)；返回连接 fd 或 -1。 */
    (void)listen_fd;
    return -1;
}

int tcp_connect(const char *host, uint16_t port) {
    /* TODO: socket -> 填地址(host:port) -> connect；返回 fd 或 -1。 */
    (void)host;
    (void)port;
    return -1;
}

uint16_t sock_local_port(int fd) {
    /* TODO: getsockname 读回绑定地址，返回 ntohs(sin_port)；出错返回 0。 */
    (void)fd;
    return 0;
}

ssize_t sock_send_all(int fd, const void *buf, size_t len) {
    /* TODO: 循环 send，直到发出 len 字节；出错返回 -1，否则返回 len。 */
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

ssize_t sock_recv_all(int fd, void *buf, size_t len) {
    /* TODO: 循环 recv，直到收满 len 字节；
     *       recv 返回 0（对端关闭）则返回已收字节或 0；出错返回 -1。 */
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

void sock_close(int fd) {
    /* TODO: fd >= 0 时 close(fd)。 */
    (void)fd;
}

int udp_socket(uint16_t port) {
    /* TODO: socket(SOCK_DGRAM) -> bind(127.0.0.1:port)；返回 fd 或 -1。 */
    (void)port;
    return -1;
}

ssize_t udp_sendto(int fd, const char *host, uint16_t port,
                   const void *buf, size_t len) {
    /* TODO: 填目标地址(host:port)，sendto；返回发送字节数或 -1。 */
    (void)fd;
    (void)host;
    (void)port;
    (void)buf;
    (void)len;
    return -1;
}

ssize_t udp_recvfrom(int fd, void *buf, size_t len) {
    /* TODO: recvfrom(fd, buf, len, 0, NULL, NULL)；返回收到字节数或 -1。 */
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}
