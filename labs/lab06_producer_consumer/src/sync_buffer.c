/**
 * @file sync_buffer.c
 * @brief Lab 6 - 线程安全阻塞缓冲区（待你完成）
 *
 * 详见 docs/lab06_producer_consumer.md；测试：xmake lab6 test
 */
#include "sync_buffer.h"

sb_status_t sb_init(sync_buffer_t *sb, uint8_t *storage, size_t size) {
    (void)sb;
    (void)storage;
    (void)size;
    return SB_ERR_NULL;
}

void sb_destroy(sync_buffer_t *sb) {
    (void)sb;
}

sb_status_t sb_put(sync_buffer_t *sb, uint8_t byte) {
    (void)sb;
    (void)byte;
    return SB_ERR_NULL;
}

sb_status_t sb_get(sync_buffer_t *sb, uint8_t *out) {
    (void)sb;
    (void)out;
    return SB_ERR_NULL;
}

void sb_close(sync_buffer_t *sb) {
    (void)sb;
}

size_t sb_count(sync_buffer_t *sb) {
    (void)sb;
    return 0;
}
