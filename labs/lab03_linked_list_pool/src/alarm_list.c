/**
 * @file alarm_list.c
 * @brief Lab 3 - 链表 + 静态内存池实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_alarm_list.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab03_linked_list_pool.md）：
 *   - al_init：把 pool[0..capacity-1] 用 next 串成空闲链表：
 *       pool[i].next = &pool[i+1]，最后一个 next = NULL；free_head = &pool[0]。
 *       list_head = NULL，used = 0。
 *   - al_node_alloc：从 free_head 摘一个；free_head 前移；used++。
 *   - al_node_free ：把节点 next 指向 free_head；free_head = 节点；used--。
 *   - al_add：alloc 一个节点，填 code/value，追加到 list_head 尾部。
 *   - al_remove：在 list_head 链上找首个 code 匹配的节点，修正前驱 next，
 *                再 al_node_free 还回池。
 *   - 全程不使用 malloc/free。
 */
#include "alarm_list.h"

ll_status_t al_init(alarm_list_t *al, ll_node_t *storage, size_t capacity) {
    /* TODO:
     *   1. al / storage 为 NULL 或 capacity == 0 -> LL_ERR_NULL
     *   2. 绑定 pool/capacity；把所有节点串成空闲链表；
     *      list_head=NULL，used=0 -> LL_OK
     */
    (void)al;
    (void)storage;
    (void)capacity;
    return LL_ERR_NULL; /* 占位：实现前测试应当失败 */
}

void al_reset(alarm_list_t *al) {
    /* TODO: 把所有节点重新串成空闲链表，list_head=NULL，used=0。
     *       （最简单：等价于用已有的 pool/capacity 重新初始化空闲链表。） */
    (void)al;
}

size_t al_count(const alarm_list_t *al) {
    /* TODO: NULL 返回 0；否则返回 used。 */
    (void)al;
    return 0;
}

size_t al_capacity(const alarm_list_t *al) {
    /* TODO: NULL 返回 0；否则返回 capacity。 */
    (void)al;
    return 0;
}

bool al_is_full(const alarm_list_t *al) {
    /* TODO: NULL 返回 false；否则返回 used == capacity。 */
    (void)al;
    return false;
}

ll_node_t *al_node_alloc(alarm_list_t *al) {
    /* TODO:
     *   al 为 NULL 或 free_head 为 NULL -> 返回 NULL；
     *   否则摘下 free_head（free_head = free_head->next），used++，返回摘下的节点。
     */
    (void)al;
    return NULL;
}

void al_node_free(alarm_list_t *al, ll_node_t *node) {
    /* TODO:
     *   al / node 为 NULL -> 直接返回；
     *   否则 node->next = free_head; free_head = node; used--。
     */
    (void)al;
    (void)node;
}

ll_status_t al_add(alarm_list_t *al, uint16_t code, int32_t value) {
    /* TODO:
     *   al 为 NULL -> LL_ERR_NULL；
     *   al_node_alloc 失败 -> LL_ERR_FULL；
     *   填 code/value，next=NULL，追加到业务链表尾部 -> LL_OK。
     */
    (void)al;
    (void)code;
    (void)value;
    return LL_ERR_NULL;
}

bool al_find(const alarm_list_t *al, uint16_t code, int32_t *out_value) {
    /* TODO:
     *   al 为 NULL -> false；
     *   遍历 list_head，找到 code 相同的节点：
     *     out_value 非 NULL 时写入其 value，返回 true；
     *   遍历完未找到 -> false。
     */
    (void)al;
    (void)code;
    (void)out_value;
    return false;
}

ll_status_t al_remove(alarm_list_t *al, uint16_t code) {
    /* TODO:
     *   al 为 NULL -> LL_ERR_NULL；
     *   在 list_head 链上找首个 code 匹配节点（记得维护"前驱"指针）：
     *     找到则从链表摘除、al_node_free 还回池 -> LL_OK；
     *   未找到 -> LL_ERR_NOT_FOUND。
     */
    (void)al;
    (void)code;
    return LL_ERR_NULL;
}
