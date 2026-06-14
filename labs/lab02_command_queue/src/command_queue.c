/**
 * @file command_queue.c
 * @brief Lab 2 - 命令队列 / 双端队列实现（待你完成）。
 *
 * 你的任务：把下面每个函数里的 TODO 实现掉，使 test/test_command_queue.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab02_command_queue.md）：
 *   - 环形数组：队首在 head，队尾元素在 (head + count - 1) % capacity。
 *   - push_back ：写到 (head + count) % capacity，count++。
 *   - push_front：head 先"后退一格并环绕"（head==0 时回到 capacity-1），
 *                 写入新的 head，count++。
 *   - pop_front ：读 head，head 前进并环绕，count--。
 *   - pop_back  ：读 (head + count - 1) % capacity，count--（head 不变）。
 *   - 所有公开函数都要对 NULL 参数安全。
 *   - 不要使用 malloc/free：所有存储都来自 cq->buffer。
 */
#include "command_queue.h"

cq_status_t cq_init(command_queue_t *cq, cmd_t *storage, size_t capacity) {
    /* TODO:
     *   1. cq / storage 为 NULL -> CQ_ERR_NULL
     *   2. capacity == 0        -> CQ_ERR_SIZE
     *   3. 绑定 buffer/capacity，head/count 清零 -> CQ_OK
     */
    (void)cq;
    (void)storage;
    (void)capacity;
    return CQ_ERR_NULL; /* 占位：实现前测试应当失败 */
}

void cq_reset(command_queue_t *cq) {
    /* TODO: cq 非 NULL 时把 head/count 清零（保留 buffer/capacity）。 */
    (void)cq;
}

bool cq_is_empty(const command_queue_t *cq) {
    /* TODO: NULL 视为空返回 true；否则返回 count == 0。 */
    (void)cq;
    return true;
}

bool cq_is_full(const command_queue_t *cq) {
    /* TODO: NULL 返回 false；否则返回 count == capacity。 */
    (void)cq;
    return false;
}

size_t cq_count(const command_queue_t *cq) {
    /* TODO: NULL 返回 0；否则返回 count。 */
    (void)cq;
    return 0;
}

size_t cq_capacity(const command_queue_t *cq) {
    /* TODO: NULL 返回 0；否则返回 capacity。 */
    (void)cq;
    return 0;
}

cq_status_t cq_push_back(command_queue_t *cq, cmd_t cmd) {
    /* TODO:
     *   cq 为 NULL -> CQ_ERR_NULL；已满 -> CQ_ERR_FULL；
     *   写到 (head + count) % capacity，count++ -> CQ_OK。
     */
    (void)cq;
    (void)cmd;
    return CQ_ERR_NULL;
}

cq_status_t cq_push_front(command_queue_t *cq, cmd_t cmd) {
    /* TODO:
     *   cq 为 NULL -> CQ_ERR_NULL；已满 -> CQ_ERR_FULL；
     *   head 后退并环绕（head==0 时变 capacity-1），写入新 head，count++ -> CQ_OK。
     */
    (void)cq;
    (void)cmd;
    return CQ_ERR_NULL;
}

cq_status_t cq_pop_front(command_queue_t *cq, cmd_t *out) {
    /* TODO:
     *   cq 为 NULL -> CQ_ERR_NULL；为空 -> CQ_ERR_EMPTY；
     *   取 buffer[head]，out 非 NULL 时写入 *out；head 前进并环绕；count-- -> CQ_OK。
     */
    (void)cq;
    (void)out;
    return CQ_ERR_NULL;
}

cq_status_t cq_pop_back(command_queue_t *cq, cmd_t *out) {
    /* TODO:
     *   cq 为 NULL -> CQ_ERR_NULL；为空 -> CQ_ERR_EMPTY；
     *   取 buffer[(head + count - 1) % capacity]，out 非 NULL 时写入 *out；
     *   count--（head 不变）-> CQ_OK。
     */
    (void)cq;
    (void)out;
    return CQ_ERR_NULL;
}

cq_status_t cq_front(const command_queue_t *cq, cmd_t *out) {
    /* TODO:
     *   cq / out 为 NULL -> CQ_ERR_NULL；为空 -> CQ_ERR_EMPTY；
     *   *out = buffer[head] -> CQ_OK。
     */
    (void)cq;
    (void)out;
    return CQ_ERR_NULL;
}

cq_status_t cq_back(const command_queue_t *cq, cmd_t *out) {
    /* TODO:
     *   cq / out 为 NULL -> CQ_ERR_NULL；为空 -> CQ_ERR_EMPTY；
     *   *out = buffer[(head + count - 1) % capacity] -> CQ_OK。
     */
    (void)cq;
    (void)out;
    return CQ_ERR_NULL;
}
