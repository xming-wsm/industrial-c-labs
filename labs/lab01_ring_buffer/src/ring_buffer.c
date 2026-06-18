/**
 * @file ring_buffer.c
 * @brief Lab 1 - 环形缓冲区（待你完成）
 *
 * 详见 docs/lab01_ring_buffer.md；测试：xmake lab1 test
 */
#include "ring_buffer.h"

rb_status_t rb_init(ring_buffer_t *rb, uint8_t *storage, size_t size) {
    if (rb == NULL || storage == NULL) {
        return RB_ERR_NULL;
    }

    if (size <= 0) {
        return RB_ERR_SIZE;
    }

    rb->buffer = storage;
    rb->capacity = size;
    rb->tail = 0;
    rb->head = 0;
    rb->count = 0;

    return RB_OK;
}

void rb_reset(ring_buffer_t *rb) {
    if (rb == NULL) {
        return;
    }

    rb->count = 0;
    rb->head = 0;
    rb->tail = 0;
}

bool rb_is_empty(const ring_buffer_t *rb) {
    if (rb == NULL) {
        return true;
    }
    return rb->count == 0;
}

bool rb_is_full(const ring_buffer_t *rb) {
    if (rb == NULL) {
        return false;
    }

    return rb->count == rb->capacity;
}

size_t rb_count(const ring_buffer_t *rb) {
    if (rb == NULL) {
        return 0;
    }
    return rb->count;
}

size_t rb_capacity(const ring_buffer_t *rb) {
    if (rb == NULL) {
        return 0;
    }
    return rb->capacity;
}

rb_status_t rb_push(ring_buffer_t *rb, uint8_t byte) {
    if (rb == NULL) {
        return RB_ERR_NULL;
    }

    if (rb_is_full(rb)) {
        return RB_ERR_FULL;
    }

    rb->buffer[rb->head] = byte;
    rb->head = (rb->head + 1) % rb->capacity;
    rb->count ++;

    return RB_OK;
}

rb_status_t rb_pop(ring_buffer_t *rb, uint8_t *out) {
    if (rb == NULL) {
        return RB_ERR_NULL;
    }

    if (rb_is_empty(rb)) {
        return RB_ERR_EMPTY;
    }

    // if out is null, means drop this byte
    if (out != NULL)
        *out = rb->buffer[rb->tail];

    rb->tail  = (rb->tail + 1) % rb->capacity;
    rb->count --;

    return RB_OK;
}

size_t rb_write(ring_buffer_t *rb, const uint8_t *data, size_t len) {
    if (rb == NULL || data == NULL) {
        return 0;
    }
    
    size_t cnt = 0;
    for (size_t i = 0; i < len; i ++) {
        if (!rb_is_full(rb)) {
            rb_push(rb, data[i]);
            cnt ++;
        }
        else {
            break;
        }
    }

    return cnt;
}

size_t rb_read(ring_buffer_t *rb, uint8_t *out, size_t len) {
    if (rb == NULL || out == NULL) {
        return 0;
    }

    size_t cnt = 0;
    for (size_t i = 0; i < len; i ++) {
        if (!rb_is_empty(rb)) {
            rb_pop(rb, out + i);
            cnt ++;
        }
        else {
            break;
        }
    }

    return cnt;
}
