/**
 * @file socket_comm.c
 * @brief Lab 9 - TCP/UDP socket 封装（待你完成）
 *
 * 详见 docs/lab09_socket_comm.md；测试：xmake lab9 test
 */
#include "socket_comm.h"

int tcp_listen(uint16_t port, int backlog) {
    (void)port;
    (void)backlog;
    return -1;
}

int tcp_accept(int listen_fd) {
    (void)listen_fd;
    return -1;
}

int tcp_connect(const char *host, uint16_t port) {
    (void)host;
    (void)port;
    return -1;
}

uint16_t sock_local_port(int fd) {
    (void)fd;
    return 0;
}

uint16_t sock_peer_port(int fd) {
    (void)fd;
    return 0;
}

ssize_t sock_send_all(int fd, const void *buf, size_t len) {
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

ssize_t sock_recv_all(int fd, void *buf, size_t len) {
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

void sock_close(int fd) {
    (void)fd;
}

int udp_socket(uint16_t port) {
    (void)port;
    return -1;
}

ssize_t udp_sendto(int fd, const char *host, uint16_t port,
                   const void *buf, size_t len) {
    (void)fd;
    (void)host;
    (void)port;
    (void)buf;
    (void)len;
    return -1;
}

ssize_t udp_recvfrom(int fd, void *buf, size_t len) {
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

ssize_t udp_recvfrom_from(int fd, void *buf, size_t len,
                          char *out_host, size_t host_cap, uint16_t *out_port) {
    (void)fd;
    (void)buf;
    (void)len;
    (void)out_host;
    (void)host_cap;
    (void)out_port;
    return -1;
}
