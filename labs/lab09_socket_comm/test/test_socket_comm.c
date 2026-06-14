/**
 * @file test_socket_comm.c
 * @brief Lab 9 测试套件。无需修改本文件；实现 socket_comm.c 使其全部通过即可。
 *
 * 全部在 127.0.0.1 回环上自收自发；服务端跑在子线程里。
 */
#include "test_framework.h"
#include "socket_comm.h"
#include <pthread.h>
#include <string.h>

/* ---- TCP echo 服务端线程 ---- */

typedef struct {
    int   listen_fd;
    int   ok;
    char  recv_buf[64];
    size_t expect_len;
} echo_arg_t;

static void *tcp_echo_server(void *p) {
    echo_arg_t *arg = (echo_arg_t *)p;
    arg->ok = 0;
    int cfd = tcp_accept(arg->listen_fd);
    if (cfd < 0) return NULL;
    /* 收满 expect_len 字节后原样回发 */
    ssize_t n = sock_recv_all(cfd, arg->recv_buf, arg->expect_len);
    if (n == (ssize_t)arg->expect_len) {
        if (sock_send_all(cfd, arg->recv_buf, arg->expect_len) == (ssize_t)arg->expect_len)
            arg->ok = 1;
    }
    sock_close(cfd);
    return NULL;
}

static int test_tcp_echo(void) {
    const char *msg = "INJ:START;pressure=120;speed=80";
    size_t len = strlen(msg);

    int lfd = tcp_listen(0, 1);     /* 端口 0：内核分配 */
    ASSERT_TRUE(lfd >= 0);
    uint16_t port = sock_local_port(lfd);
    ASSERT_TRUE(port != 0);

    echo_arg_t arg;
    arg.listen_fd = lfd;
    arg.expect_len = len;
    pthread_t srv;
    ASSERT_EQ_INT(0, pthread_create(&srv, NULL, tcp_echo_server, &arg));

    int cfd = tcp_connect("127.0.0.1", port);
    ASSERT_TRUE(cfd >= 0);
    ASSERT_EQ_INT((int)len, (int)sock_send_all(cfd, msg, len));

    char reply[64] = {0};
    ASSERT_EQ_INT((int)len, (int)sock_recv_all(cfd, reply, len));
    ASSERT_EQ_MEM(msg, reply, len);

    sock_close(cfd);
    pthread_join(srv, NULL);
    sock_close(lfd);

    ASSERT_TRUE(arg.ok == 1);
    return 0;
}

/* ---- 大数据量：验证 send_all/recv_all 正确处理"部分收发" ---- */

#define BIG_LEN 200000u

typedef struct {
    int listen_fd;
    int ok;
} drain_arg_t;

static void *tcp_drain_server(void *p) {
    drain_arg_t *arg = (drain_arg_t *)p;
    arg->ok = 0;
    int cfd = tcp_accept(arg->listen_fd);
    if (cfd < 0) return NULL;
    static unsigned char buf[BIG_LEN];
    ssize_t n = sock_recv_all(cfd, buf, BIG_LEN);
    if (n == (ssize_t)BIG_LEN) {
        int good = 1;
        for (size_t i = 0; i < BIG_LEN; i++) {
            if (buf[i] != (unsigned char)(i & 0xFF)) { good = 0; break; }
        }
        arg->ok = good;
    }
    sock_close(cfd);
    return NULL;
}

static int test_tcp_big_transfer(void) {
    int lfd = tcp_listen(0, 1);
    ASSERT_TRUE(lfd >= 0);
    uint16_t port = sock_local_port(lfd);
    ASSERT_TRUE(port != 0);

    drain_arg_t arg;
    arg.listen_fd = lfd;
    pthread_t srv;
    ASSERT_EQ_INT(0, pthread_create(&srv, NULL, tcp_drain_server, &arg));

    int cfd = tcp_connect("127.0.0.1", port);
    ASSERT_TRUE(cfd >= 0);

    static unsigned char out[BIG_LEN];
    for (size_t i = 0; i < BIG_LEN; i++) out[i] = (unsigned char)(i & 0xFF);
    ASSERT_EQ_INT((int)BIG_LEN, (int)sock_send_all(cfd, out, BIG_LEN));

    sock_close(cfd);
    pthread_join(srv, NULL);
    sock_close(lfd);

    ASSERT_TRUE(arg.ok == 1);
    return 0;
}

/* ---- UDP 收发 ---- */

static int test_udp_sendrecv(void) {
    const char *msg = "STATUS:RUN;cycle=42";
    size_t len = strlen(msg);

    int rfd = udp_socket(0);    /* 接收端，内核分配端口 */
    ASSERT_TRUE(rfd >= 0);
    uint16_t port = sock_local_port(rfd);
    ASSERT_TRUE(port != 0);

    int sfd = udp_socket(0);    /* 发送端 */
    ASSERT_TRUE(sfd >= 0);

    ASSERT_EQ_INT((int)len, (int)udp_sendto(sfd, "127.0.0.1", port, msg, len));

    char buf[64] = {0};
    ssize_t n = udp_recvfrom(rfd, buf, sizeof(buf));
    ASSERT_EQ_INT((int)len, (int)n);
    ASSERT_EQ_MEM(msg, buf, len);

    sock_close(sfd);
    sock_close(rfd);
    return 0;
}

/* ---- 错误路径 ---- */

static int test_error_paths(void) {
    /* 连接一个几乎不可能在监听的端口应失败。 */
    int fd = tcp_connect("127.0.0.1", 1);   /* 端口 1 通常无人监听 */
    if (fd >= 0) sock_close(fd);             /* 万一环境特殊，至少别泄漏 */
    ASSERT_TRUE(fd < 0);

    ASSERT_EQ_INT(0, (int)sock_local_port(-1));
    sock_close(-1);  /* 不应崩溃 */
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_tcp_echo);
    RUN_TEST(test_tcp_big_transfer);
    RUN_TEST(test_udp_sendrecv);
    RUN_TEST(test_error_paths);
    TEST_END();
}
