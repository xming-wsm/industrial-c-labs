/**
 * @file test_framework.h
 * @brief 极简单元测试框架（header-only），用于 Industrial C Labs。
 *
 * 设计目标：
 *   - 零外部依赖，对新手完全透明（你可以读懂每一行）；
 *   - 一个测试用例 = 一个返回 int 的函数（0 = 通过，非 0 = 失败）；
 *   - 失败时打印文件:行号 + 表达式，定位问题；
 *   - main 里用 RUN_TEST(fn) 注册并执行，整体失败则进程返回非 0，
 *     `xmake labN test` 据此判定 PASS / FAIL。
 *
 * 用法：
 *   #include "test_framework.h"
 *
 *   static int test_something(void) {
 *       ASSERT_TRUE(1 + 1 == 2);
 *       ASSERT_EQ_INT(2, 1 + 1);
 *       return 0;            // 别忘了：通过的用例必须 return 0
 *   }
 *
 *   int main(void) {
 *       TEST_BEGIN();
 *       RUN_TEST(test_something);
 *       TEST_END();          // 返回 0/1 给操作系统
 *   }
 */
#ifndef LABS_TEST_FRAMEWORK_H
#define LABS_TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

/* 全局计数器（header-only：用 static 让每个测试可执行各自持有一份）。 */
static int g_tests_run = 0;
static int g_tests_failed = 0;

/* 标记当前用例失败（在断言宏内部使用）。 */
#define TF__FAIL_PRINT(fmt, ...)                                            \
    do {                                                                    \
        fprintf(stderr, "  [FAIL] %s:%d: " fmt "\n",                        \
                __FILE__, __LINE__, ##__VA_ARGS__);                         \
    } while (0)

/* ---- 断言宏：失败即打印并 return -1（结束当前用例） ---- */

#define ASSERT_TRUE(expr)                                                   \
    do {                                                                    \
        if (!(expr)) {                                                      \
            TF__FAIL_PRINT("ASSERT_TRUE(%s)", #expr);                       \
            return -1;                                                      \
        }                                                                   \
    } while (0)

#define ASSERT_FALSE(expr)                                                  \
    do {                                                                    \
        if ((expr)) {                                                       \
            TF__FAIL_PRINT("ASSERT_FALSE(%s)", #expr);                      \
            return -1;                                                      \
        }                                                                   \
    } while (0)

/* 整型相等：失败时打印期望值与实际值。 */
#define ASSERT_EQ_INT(expected, actual)                                     \
    do {                                                                    \
        long long _e = (long long)(expected);                              \
        long long _a = (long long)(actual);                                \
        if (_e != _a) {                                                     \
            TF__FAIL_PRINT("ASSERT_EQ_INT(%s, %s): expected %lld, got %lld",\
                           #expected, #actual, _e, _a);                     \
            return -1;                                                      \
        }                                                                   \
    } while (0)

/* size_t / 无符号长度相等。 */
#define ASSERT_EQ_SIZE(expected, actual)                                    \
    do {                                                                    \
        size_t _e = (size_t)(expected);                                    \
        size_t _a = (size_t)(actual);                                      \
        if (_e != _a) {                                                     \
            TF__FAIL_PRINT("ASSERT_EQ_SIZE(%s, %s): expected %zu, got %zu", \
                           #expected, #actual, _e, _a);                     \
            return -1;                                                      \
        }                                                                   \
    } while (0)

/* 内存块相等（用于比较缓冲区内容）。 */
#define ASSERT_EQ_MEM(expected, actual, n)                                  \
    do {                                                                    \
        if (memcmp((expected), (actual), (n)) != 0) {                       \
            TF__FAIL_PRINT("ASSERT_EQ_MEM(%s, %s, %s): buffers differ",     \
                           #expected, #actual, #n);                         \
            return -1;                                                      \
        }                                                                   \
    } while (0)

/* ---- 运行器 ---- */

#define TEST_BEGIN()                                                        \
    do {                                                                    \
        g_tests_run = 0;                                                    \
        g_tests_failed = 0;                                                 \
        printf("==== %s ====\n", __FILE__);                                 \
    } while (0)

/* 执行一个用例：fn() 返回 0 视为通过。 */
#define RUN_TEST(fn)                                                        \
    do {                                                                    \
        printf("-- %s\n", #fn);                                             \
        g_tests_run++;                                                      \
        if ((fn)() != 0) {                                                  \
            g_tests_failed++;                                               \
            printf("   => FAILED\n");                                       \
        } else {                                                            \
            printf("   => ok\n");                                           \
        }                                                                   \
    } while (0)

/* 打印汇总并返回退出码：全部通过返回 0，否则返回 1。 */
#define TEST_END()                                                          \
    do {                                                                    \
        printf("==== summary: %d run, %d failed ====\n",                    \
               g_tests_run, g_tests_failed);                                \
        return (g_tests_failed == 0) ? 0 : 1;                               \
    } while (0)

#endif /* LABS_TEST_FRAMEWORK_H */
