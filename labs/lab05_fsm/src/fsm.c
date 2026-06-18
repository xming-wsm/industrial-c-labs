/**
 * @file fsm.c
 * @brief Lab 5 - 表驱动有限状态机（待你完成）
 *
 * 详见 docs/lab05_fsm.md；测试：xmake lab5 test
 */
#include "fsm.h"

fsm_status_t fsm_init(fsm_t *fsm,
                      const fsm_transition_t *table, size_t table_len,
                      int initial, void *ctx) {
    (void)fsm;
    (void)table;
    (void)table_len;
    (void)initial;
    (void)ctx;
    return FSM_ERR_NULL;
}

int fsm_state(const fsm_t *fsm) {
    (void)fsm;
    return -1;
}

fsm_status_t fsm_dispatch(fsm_t *fsm, int event) {
    (void)fsm;
    (void)event;
    return FSM_ERR_NULL;
}
