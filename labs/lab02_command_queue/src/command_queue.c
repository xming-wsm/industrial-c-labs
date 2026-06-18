/**
 * @file command_queue.c
 * @brief Lab 2 - 命令队列 / 双端队列（待你完成）
 *
 * 详见 docs/lab02_command_queue.md；测试：xmake lab2 test
 */
#include "command_queue.h"

cq_status_t cq_init(command_queue_t *cq, cmd_t *storage, size_t capacity) {
    (void)cq;
    (void)storage;
    (void)capacity;
    return CQ_ERR_NULL;
}

void cq_reset(command_queue_t *cq) {
    (void)cq;
}

bool cq_is_empty(const command_queue_t *cq) {
    (void)cq;
    return true;
}

bool cq_is_full(const command_queue_t *cq) {
    (void)cq;
    return false;
}

size_t cq_count(const command_queue_t *cq) {
    (void)cq;
    return 0;
}

size_t cq_capacity(const command_queue_t *cq) {
    (void)cq;
    return 0;
}

cq_status_t cq_push_back(command_queue_t *cq, cmd_t cmd) {
    (void)cq;
    (void)cmd;
    return CQ_ERR_NULL;
}

cq_status_t cq_push_front(command_queue_t *cq, cmd_t cmd) {
    (void)cq;
    (void)cmd;
    return CQ_ERR_NULL;
}

cq_status_t cq_pop_front(command_queue_t *cq, cmd_t *out) {
    (void)cq;
    (void)out;
    return CQ_ERR_NULL;
}

cq_status_t cq_pop_back(command_queue_t *cq, cmd_t *out) {
    (void)cq;
    (void)out;
    return CQ_ERR_NULL;
}

cq_status_t cq_front(const command_queue_t *cq, cmd_t *out) {
    (void)cq;
    (void)out;
    return CQ_ERR_NULL;
}

cq_status_t cq_back(const command_queue_t *cq, cmd_t *out) {
    (void)cq;
    (void)out;
    return CQ_ERR_NULL;
}
