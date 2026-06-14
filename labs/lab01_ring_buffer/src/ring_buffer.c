/**
 * @file ring_buffer.c
 * @brief Lab 1 - 环形缓冲区实现（待你完成）。
 *
 * 你的任务：把下面每个函数里的 TODO 实现掉，使 test/test_ring_buffer.c 全部通过。
 *
 * 构建并测试：
 *   cmake -S . -B build
 *   cmake --build build
 *   ctest --test-dir build --output-on-failure
 *
 * 实现要点（强烈建议先读 docs/lab01_ring_buffer.md）：
 *   - 游标环绕：写/读后游标自增，等于 capacity 时回绕到 0。
 *     既可以用取模 (idx % capacity)，也可以用 if (idx == capacity) idx = 0;
 *   - 满 / 空判定：本实现保存了 count 字段，
 *       空  <=> count == 0
 *       满  <=> count == capacity
 *   - 所有公开函数都要对 NULL 参数安全（见头文件里每个函数的约定）。
 *   - 不要使用 malloc/free：所有存储都来自 rb->buffer。
 */
#include "ring_buffer.h"

rb_status_t rb_init(ring_buffer_t *rb, uint8_t *storage, size_t size) {
    /* TODO:
     *   1. 检查 rb / storage 是否为 NULL -> RB_ERR_NULL
     *   2. 检查 size 是否为 0           -> RB_ERR_SIZE
     *   3. 绑定 buffer/capacity，并把 head/tail/count 清零 -> RB_OK
     */
    (void)rb;
    (void)storage;
    (void)size;
    return RB_ERR_NULL; /* 占位：实现前测试应当失败 */
}

void rb_reset(ring_buffer_t *rb) {
    /* TODO: rb 非 NULL 时，把 head/tail/count 清零（保留 buffer/capacity）。 */
    (void)rb;
}

bool rb_is_empty(const ring_buffer_t *rb) {
    /* TODO: NULL 视为空返回 true；否则返回 count == 0。 */
    (void)rb;
    return true;
}

bool rb_is_full(const ring_buffer_t *rb) {
    /* TODO: NULL 返回 false；否则返回 count == capacity。 */
    (void)rb;
    return false;
}

size_t rb_count(const ring_buffer_t *rb) {
    /* TODO: NULL 返回 0；否则返回 count。 */
    (void)rb;
    return 0;
}

size_t rb_capacity(const ring_buffer_t *rb) {
    /* TODO: NULL 返回 0；否则返回 capacity。 */
    (void)rb;
    return 0;
}

rb_status_t rb_push(ring_buffer_t *rb, uint8_t byte) {
    /* TODO:
     *   1. rb 为 NULL -> RB_ERR_NULL
     *   2. 已满       -> RB_ERR_FULL
     *   3. buffer[head] = byte; head 前进并环绕; count++ -> RB_OK
     */
    (void)rb;
    (void)byte;
    return RB_ERR_NULL;
}

rb_status_t rb_pop(ring_buffer_t *rb, uint8_t *out) {
    /* TODO:
     *   1. rb 为 NULL -> RB_ERR_NULL
     *   2. 为空       -> RB_ERR_EMPTY
     *   3. 取 buffer[tail]，若 out 非 NULL 则写入 *out;
     *      tail 前进并环绕; count-- -> RB_OK
     */
    (void)rb;
    (void)out;
    return RB_ERR_NULL;
}

size_t rb_write(ring_buffer_t *rb, const uint8_t *data, size_t len) {
    /* TODO:
     *   rb/data 为 NULL -> 返回 0。
     *   循环调用 rb_push 写入，直到写满或写完 len 个；返回实际写入数。
     */
    (void)rb;
    (void)data;
    (void)len;
    return 0;
}

size_t rb_read(ring_buffer_t *rb, uint8_t *out, size_t len) {
    /* TODO:
     *   rb/out 为 NULL -> 返回 0。
     *   循环调用 rb_pop 读出，直到读空或读够 len 个；返回实际读出数。
     */
    (void)rb;
    (void)out;
    (void)len;
    return 0;
}
