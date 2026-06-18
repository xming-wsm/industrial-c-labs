/**
 * @file alarm_list.c
 * @brief Lab 3 - 链表 + 静态内存池（待你完成）
 *
 * 详见 docs/lab03_linked_list_pool.md；测试：xmake lab3 test
 */
#include "alarm_list.h"

ll_status_t al_init(alarm_list_t *al, ll_node_t *storage, size_t capacity) {
    (void)al;
    (void)storage;
    (void)capacity;
    return LL_ERR_NULL;
}

void al_reset(alarm_list_t *al) {
    (void)al;
}

size_t al_count(const alarm_list_t *al) {
    (void)al;
    return 0;
}

size_t al_capacity(const alarm_list_t *al) {
    (void)al;
    return 0;
}

bool al_is_full(const alarm_list_t *al) {
    (void)al;
    return false;
}

ll_node_t *al_node_alloc(alarm_list_t *al) {
    (void)al;
    return NULL;
}

void al_node_free(alarm_list_t *al, ll_node_t *node) {
    (void)al;
    (void)node;
}

ll_status_t al_add(alarm_list_t *al, uint16_t code, int32_t value) {
    (void)al;
    (void)code;
    (void)value;
    return LL_ERR_NULL;
}

bool al_find(const alarm_list_t *al, uint16_t code, int32_t *out_value) {
    (void)al;
    (void)code;
    (void)out_value;
    return false;
}

ll_status_t al_remove(alarm_list_t *al, uint16_t code) {
    (void)al;
    (void)code;
    return LL_ERR_NULL;
}
