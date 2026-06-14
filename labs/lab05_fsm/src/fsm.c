/**
 * @file fsm.c
 * @brief Lab 5 - 表驱动有限状态机引擎实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_fsm.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab05_fsm.md）：
 *   - 引擎不关心状态/事件的"含义"，只做查表 + 跳转。
 *   - fsm_dispatch：线性扫描 table，找第一条 from==state && event==e 的规则：
 *       命中 -> state = to; 若 action 非 NULL 则 action(ctx); 返回 FSM_OK。
 *       扫完没命中 -> 返回 FSM_NO_TRANSITION（state 不变）。
 *   - 全程不使用 malloc/free。
 */
#include "fsm.h"

fsm_status_t fsm_init(fsm_t *fsm,
                      const fsm_transition_t *table, size_t table_len,
                      int initial, void *ctx) {
    /* TODO:
     *   1. fsm/table 为 NULL 或 table_len==0 -> FSM_ERR_NULL
     *   2. 绑定 table/table_len/ctx，state = initial -> FSM_OK
     */
    (void)fsm;
    (void)table;
    (void)table_len;
    (void)initial;
    (void)ctx;
    return FSM_ERR_NULL; /* 占位：实现前测试应当失败 */
}

int fsm_state(const fsm_t *fsm) {
    /* TODO: NULL 返回 -1；否则返回 state。 */
    (void)fsm;
    return -1;
}

fsm_status_t fsm_dispatch(fsm_t *fsm, int event) {
    /* TODO:
     *   fsm 为 NULL -> FSM_ERR_NULL。
     *   遍历 table[0..table_len-1]：
     *     若 table[i].from == fsm->state 且 table[i].event == event：
     *        fsm->state = table[i].to;
     *        若 table[i].action 非 NULL，调用 table[i].action(fsm->ctx);
     *        返回 FSM_OK。
     *   未命中 -> FSM_NO_TRANSITION。
     */
    (void)fsm;
    (void)event;
    return FSM_ERR_NULL;
}
