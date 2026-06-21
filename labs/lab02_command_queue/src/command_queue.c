/**
 * @file command_queue.c
 * @brief Lab 2 - 命令队列 / 双端队列（待你完成）
 *
 * 详见 docs/lab02_command_queue.md；测试：xmake lab2 test
 */
#include "command_queue.h"
#include <string.h>

cq_status_t cq_init(command_queue_t *cq, cmd_t *storage, size_t capacity) {
    if (cq == NULL) 
        return CQ_ERR_NULL;

    if (storage == NULL) 
        return CQ_ERR_NULL;

    if (capacity <= 0) 
        return CQ_ERR_SIZE;


    cq->buffer = storage;
    cq->capacity = capacity;
    cq->count = 0;
    cq->head = 0;

    return CQ_OK;
}

void cq_reset(command_queue_t *cq) {
    if (cq == NULL)
        return;

    memset(cq->buffer, 0, sizeof(cmd_t) * cq->capacity);
    cq->count = 0;
    cq->head = 0;
}

bool cq_is_empty(const command_queue_t *cq) {
    if (cq == NULL)
        return true;
    return cq->count == 0;
}

bool cq_is_full(const command_queue_t *cq) {
    if (cq == NULL)
        return false;
    return cq->count == cq->capacity;
}

size_t cq_count(const command_queue_t *cq) {
    if (cq == NULL)
        return 0;
    return cq->count;
}

size_t cq_capacity(const command_queue_t *cq) {
    if (cq == NULL)
        return 0;
    return cq->capacity;
}

cq_status_t cq_push_back(command_queue_t *cq, cmd_t cmd) {
    if (cq == NULL) 
        return CQ_ERR_NULL;

    if (cq_is_full(cq))
        return CQ_ERR_FULL;

    size_t tail = (cq->head + cq->count) % cq->capacity;
    cq->buffer[tail] = cmd;
    cq->count ++;
    return CQ_OK;
}

cq_status_t cq_push_front(command_queue_t *cq, cmd_t cmd) {
    if (cq == NULL) 
        return CQ_ERR_NULL;

    if (cq_is_full(cq))
        return CQ_ERR_FULL;

    cq->head = (cq->head == 0) ? (cq->capacity - 1) : (cq->head - 1);
    cq->buffer[cq->head] = cmd;
    cq->count ++;
    return CQ_OK;
}

cq_status_t cq_pop_front(command_queue_t *cq, cmd_t *out) {
    if (cq == NULL)
        return CQ_ERR_NULL;

    if (cq_is_empty(cq))
        return CQ_ERR_EMPTY;

    if (out == NULL) {
        cq->count --;    
        cq->head = (cq->head == cq->capacity - 1) ? 0 : cq->head + 1;
        return CQ_OK;
    }

    *out = cq->buffer[cq->head];
    cq->head = (cq->head == cq->capacity - 1) ? 0 : cq->head + 1;
    cq->count --;
    return CQ_OK;
}

cq_status_t cq_pop_back(command_queue_t *cq, cmd_t *out) {
    if (cq == NULL)
        return CQ_ERR_NULL;

    if (cq_is_empty(cq))
        return CQ_ERR_EMPTY;

    if (out == NULL) {
        cq->count --;    
        return CQ_OK;
    }

    size_t tail = (cq->head + cq->count - 1) % cq->capacity;
    *out = cq->buffer[tail];
    cq->count --;
    return CQ_OK;
}


cq_status_t cq_front(const command_queue_t *cq, cmd_t *out) {
    if (cq == NULL || out == NULL)
        return CQ_ERR_NULL;

    if (cq_is_empty(cq))
        return CQ_ERR_EMPTY;

    *out = cq->buffer[cq->head];
    return CQ_OK;
}

cq_status_t cq_back(const command_queue_t *cq, cmd_t *out) {
    if (cq == NULL || out == NULL)
        return CQ_ERR_NULL;

    if (cq_is_empty(cq))
        return CQ_ERR_EMPTY;

    size_t tail = (cq->head + cq->count - 1) % cq->capacity;
    *out = cq->buffer[tail];
    return CQ_OK;
}
