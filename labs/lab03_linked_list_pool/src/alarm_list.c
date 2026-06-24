/**
 * @file alarm_list.c
 * @brief Lab 3 - 链表 + 静态内存池（待你完成）
 *
 * 详见 docs/lab03_linked_list_pool.md；测试：xmake lab3 test
 */
#include "alarm_list.h"

ll_status_t al_init(alarm_list_t *al, ll_node_t *storage, size_t capacity) {
    if (al == NULL || storage == NULL || capacity == 0)
        return LL_ERR_NULL;

    al->pool = storage;
    al->capacity = capacity;
    al->used = 0;

    al->list_head = NULL;
    al->free_head = storage;
    ll_node_t* node = al->free_head;
    for (size_t i = 1; i < capacity; i ++) {
        node->next = storage + i;
        node = node->next;
    }

    return LL_OK;
}

void al_reset(alarm_list_t *al) {
    if (al == NULL)
        return;

    for (size_t i = 0; i < al->capacity; i ++) {
        al->pool[i].code  = 0;
        al->pool[i].value = 0;
        al->pool[i].next  = NULL;
    }

    al->free_head = al->pool;
    ll_node_t* node = al->free_head;
    for (size_t i = 1; i < al->capacity; i ++) {
        node->next = al->pool + i;
        node = node->next;
    }
    al->used = 0;

}

size_t al_count(const alarm_list_t *al) {
    if (al == NULL)
        return 0;
    return al->used;
}

size_t al_capacity(const alarm_list_t *al) {
    if (al == NULL)
        return 0;
    return al->capacity;
}

bool al_is_full(const alarm_list_t *al) {
    if (al == NULL)
        return false;
    return al->used == al->capacity;
}

size_t al_available(const alarm_list_t *al) {
    if (al == NULL)
        return 0;
    
    return al->capacity - al->used; 
}

ll_node_t *al_node_alloc(alarm_list_t *al) {
    if (al == NULL)
        return NULL;

    size_t fn_num = al_available(al);
    if (fn_num) {
        ll_node_t* tmp = al->free_head;
        al->free_head = al->free_head->next;
        al->used ++;
        return tmp;
    }
    else {
        return NULL;
    }
}

void al_node_free(alarm_list_t *al, ll_node_t *node) {
    if (al == NULL || node == NULL)
        return;

    node->next = al->free_head;
    al->free_head = node;
    al->used --;
}

ll_status_t al_add(alarm_list_t *al, uint16_t code, int32_t value) {
    if (al == NULL)
        return LL_ERR_NULL;

    if (al_is_full(al))
        return LL_ERR_FULL;

    ll_node_t* node = al_node_alloc(al);
    node->value = value;
    node->code = code;

    node->next = al->list_head;
    al->list_head = node;
    return LL_OK;
}

bool al_find(const alarm_list_t *al, uint16_t code, int32_t *out_value) {
    if (al == NULL)
        return false;

    ll_node_t* n = al->list_head;
    while (n != NULL) {
        if (n->code == code) {
            if (out_value) {
                *out_value = n->value;
            }
            return true;
        }
        n = n->next;
    }

    return false;
}

ll_status_t al_update(alarm_list_t *al, uint16_t code, int32_t value) {
    if (al == NULL)
        return LL_ERR_NULL;

    ll_node_t* n = al->list_head;
    while (n != NULL) {
        if (n->code == code) {
            n->value = value;
            return LL_OK;
        }
        n = n->next;
    }

    return LL_ERR_NOT_FOUND;
}

ll_status_t al_remove(alarm_list_t *al, uint16_t code) {
    if (al == NULL)
        return LL_ERR_NULL;

    ll_node_t* n = al->list_head;
    ll_node_t* nn = n->next;

    if (n->code == code) {
        al->list_head = nn;
        al_node_free(al, n);
        return LL_OK;
    }

    while (nn != NULL) {
        if (nn->code == code) {
            n->next = nn->next;
            al_node_free(al, nn);
            return LL_OK;
        }
        n = n->next;
        nn = nn->next;
    }

    return LL_ERR_NOT_FOUND;
}

size_t al_remove_all(alarm_list_t *al, uint16_t code) {
    size_t cnt = 0;
    while (al_remove(al, code) == LL_OK) {
        cnt ++;
    }
    return cnt;
}

void al_for_each(const alarm_list_t *al, al_visit_fn fn, void *ctx) {
    if (al == NULL || fn == NULL)
        return;
    
    ll_node_t* n = al->list_head;
    while (n != NULL) {
       fn(n->code, n->value, ctx);
       n = n->next;
    }
}
