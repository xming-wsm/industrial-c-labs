/**
 * @file sync_buffer.c
 * @brief Lab 6 - 线程安全阻塞缓冲区实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_sync_buffer.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab06_producer_consumer.md）：
 *   - sb_init：除了绑定存储区/清零游标，还要
 *       pthread_mutex_init(&sb->lock, NULL);
 *       pthread_cond_init(&sb->not_empty, NULL);
 *       pthread_cond_init(&sb->not_full, NULL);
 *       sb->closed = false;
 *   - sb_put：加锁；while (满 且 未关闭) pthread_cond_wait(&not_full, &lock);
 *            若已关闭 -> 解锁返回 SB_CLOSED；
 *            否则写入、count++、pthread_cond_signal(&not_empty)；解锁返回 SB_OK。
 *   - sb_get：加锁；while (空 且 未关闭) pthread_cond_wait(&not_empty, &lock);
 *            若空 且 已关闭 -> 解锁返回 SB_CLOSED；
 *            否则读出、count--、pthread_cond_signal(&not_full)；解锁返回 SB_OK。
 *   - sb_close：加锁；closed=true；pthread_cond_broadcast 两个条件变量；解锁。
 *   - 关键：用 while（不是 if）重新检查条件，防止虚假唤醒。
 */
#include "sync_buffer.h"

sb_status_t sb_init(sync_buffer_t *sb, uint8_t *storage, size_t size) {
    /* TODO:
     *   1. sb/storage 为 NULL -> SB_ERR_NULL；size==0 -> SB_ERR_SIZE
     *   2. 绑定 buffer/capacity，head/tail/count=0，closed=false
     *   3. 初始化 mutex 与两个 cond -> SB_OK
     */
    (void)sb;
    (void)storage;
    (void)size;
    return SB_ERR_NULL; /* 占位：实现前测试应当失败 */
}

void sb_destroy(sync_buffer_t *sb) {
    /* TODO: sb 非 NULL 时销毁 mutex 与两个 cond。 */
    (void)sb;
}

sb_status_t sb_put(sync_buffer_t *sb, uint8_t byte) {
    /* TODO: 见文件头"实现要点"。注意用 while 重判条件、配对解锁。 */
    (void)sb;
    (void)byte;
    return SB_ERR_NULL;
}

sb_status_t sb_get(sync_buffer_t *sb, uint8_t *out) {
    /* TODO: 见文件头"实现要点"。注意"已关闭且已排空"才返回 SB_CLOSED。 */
    (void)sb;
    (void)out;
    return SB_ERR_NULL;
}

void sb_close(sync_buffer_t *sb) {
    /* TODO: 加锁置 closed=true，broadcast 两个条件变量，解锁。 */
    (void)sb;
}

size_t sb_count(sync_buffer_t *sb) {
    /* TODO: NULL 返回 0；否则加锁读 count 再解锁返回。 */
    (void)sb;
    return 0;
}
