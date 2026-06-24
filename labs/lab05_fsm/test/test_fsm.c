/**
 * @file test_fsm.c
 * @brief Lab 5 测试套件。无需修改本文件；实现 fsm.c 使其全部通过即可。
 *
 * 本文件用注塑周期定义一张转移表，验证引擎的查表/跳转/动作回调。
 */
#include "test_framework.h"
#include "fsm.h"

/* ---- 注塑周期：状态 ---- */
enum {
    ST_IDLE = 0,
    ST_CLAMP,    /* 合模 */
    ST_INJECT,   /* 射胶 */
    ST_HOLD,     /* 保压 */
    ST_CHARGE,   /* 储料 */
    ST_COOL,     /* 冷却 */
    ST_OPEN,     /* 开模 */
    ST_EJECT,    /* 顶出 */
    ST_FAULT     /* 故障（急停后） */
};

/* ---- 事件 ---- */
enum {
    EV_START = 0,
    EV_CLAMP_DONE,
    EV_INJECT_DONE,
    EV_HOLD_DONE,
    EV_CHARGE_DONE,
    EV_COOL_DONE,
    EV_OPEN_DONE,
    EV_EJECT_DONE,
    EV_ESTOP,
    EV_RESET
};

/* 业务上下文：统计动作触发次数。 */
typedef struct {
    int cycle_done;   /* 完成一个完整周期的次数 */
    int estop_count;  /* 急停动作次数           */
} cycle_ctx_t;

static void on_cycle_done(void *ctx) {
    ((cycle_ctx_t *)ctx)->cycle_done++;
}

static void on_estop(void *ctx) {
    ((cycle_ctx_t *)ctx)->estop_count++;
}

/* 注塑周期转移表（顺序流程 + 任意态急停 + 故障复位）。 */
static const fsm_transition_t kCycleTable[] = {
    { ST_IDLE,   EV_START,        ST_CLAMP,  NULL },
    { ST_CLAMP,  EV_CLAMP_DONE,   ST_INJECT, NULL },
    { ST_INJECT, EV_INJECT_DONE,  ST_HOLD,   NULL },
    { ST_HOLD,   EV_HOLD_DONE,    ST_CHARGE, NULL },
    { ST_CHARGE, EV_CHARGE_DONE,  ST_COOL,   NULL },
    { ST_COOL,   EV_COOL_DONE,    ST_OPEN,   NULL },
    { ST_OPEN,   EV_OPEN_DONE,    ST_EJECT,  NULL },
    { ST_EJECT,  EV_EJECT_DONE,   ST_IDLE,   on_cycle_done },
    /* 任意工作态急停 -> 故障 */
    { ST_CLAMP,  EV_ESTOP,        ST_FAULT,  on_estop },
    { ST_INJECT, EV_ESTOP,        ST_FAULT,  on_estop },
    { ST_HOLD,   EV_ESTOP,        ST_FAULT,  on_estop },
    { ST_CHARGE, EV_ESTOP,        ST_FAULT,  on_estop },
    { ST_COOL,   EV_ESTOP,        ST_FAULT,  on_estop },
    { ST_OPEN,   EV_ESTOP,        ST_FAULT,  on_estop },
    { ST_EJECT,  EV_ESTOP,        ST_FAULT,  on_estop },
    /* 故障复位 */
    { ST_FAULT,  EV_RESET,        ST_IDLE,   NULL },
};
static const size_t kCycleLen = sizeof(kCycleTable) / sizeof(kCycleTable[0]);

/* ---- 测试 ---- */

static int test_init_basic(void) {
    fsm_t fsm;
    cycle_ctx_t ctx = {0, 0};
    ASSERT_EQ_INT(FSM_OK, fsm_init(&fsm, kCycleTable, kCycleLen, ST_IDLE, &ctx));
    ASSERT_EQ_INT(ST_IDLE, fsm_state(&fsm));
    return 0;
}

static int test_init_null(void) {
    fsm_t fsm;
    ASSERT_EQ_INT(FSM_ERR_NULL, fsm_init(NULL, kCycleTable, kCycleLen, ST_IDLE, NULL));
    ASSERT_EQ_INT(FSM_ERR_NULL, fsm_init(&fsm, NULL, kCycleLen, ST_IDLE, NULL));
    ASSERT_EQ_INT(FSM_ERR_NULL, fsm_init(&fsm, kCycleTable, 0, ST_IDLE, NULL));
    ASSERT_EQ_INT(-1, fsm_state(NULL));
    ASSERT_EQ_INT(FSM_ERR_NULL, fsm_dispatch(NULL, EV_START));
    ASSERT_EQ_INT(FSM_ERR_NULL, fsm_reset(NULL, ST_IDLE));
    ASSERT_FALSE(fsm_can_dispatch(NULL, EV_START));
    ASSERT_EQ_INT(FSM_ERR_NULL, fsm_peek_next(NULL, EV_START, NULL));
    ASSERT_EQ_SIZE(0u, fsm_transition_count(NULL));
    return 0;
}

static int test_full_cycle(void) {
    fsm_t fsm;
    cycle_ctx_t ctx = {0, 0};
    fsm_init(&fsm, kCycleTable, kCycleLen, ST_IDLE, &ctx);

    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_START));       ASSERT_EQ_INT(ST_CLAMP, fsm_state(&fsm));
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_CLAMP_DONE));  ASSERT_EQ_INT(ST_INJECT, fsm_state(&fsm));
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_INJECT_DONE)); ASSERT_EQ_INT(ST_HOLD, fsm_state(&fsm));
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_HOLD_DONE));   ASSERT_EQ_INT(ST_CHARGE, fsm_state(&fsm));
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_CHARGE_DONE)); ASSERT_EQ_INT(ST_COOL, fsm_state(&fsm));
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_COOL_DONE));   ASSERT_EQ_INT(ST_OPEN, fsm_state(&fsm));
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_OPEN_DONE));   ASSERT_EQ_INT(ST_EJECT, fsm_state(&fsm));
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_EJECT_DONE));  ASSERT_EQ_INT(ST_IDLE, fsm_state(&fsm));

    /* 顶出完成的 action 触发了一次"周期完成" */
    ASSERT_EQ_INT(1, ctx.cycle_done);
    ASSERT_EQ_INT(0, ctx.estop_count);
    /* 一个完整周期 = 8 次成功转移 */
    ASSERT_EQ_SIZE(8u, fsm_transition_count(&fsm));
    return 0;
}

/* ---- can_dispatch / peek_next：不改变状态的查询 ---- */

static int test_peek_and_can(void) {
    fsm_t fsm;
    cycle_ctx_t ctx = {0, 0};
    fsm_init(&fsm, kCycleTable, kCycleLen, ST_IDLE, &ctx);

    /* 空闲态：START 有转移，INJECT_DONE 没有 */
    ASSERT_TRUE(fsm_can_dispatch(&fsm, EV_START));
    ASSERT_FALSE(fsm_can_dispatch(&fsm, EV_INJECT_DONE));

    int next = -1;
    ASSERT_EQ_INT(FSM_OK, fsm_peek_next(&fsm, EV_START, &next));
    ASSERT_EQ_INT(ST_CLAMP, next);
    /* peek 不改变状态、不计数、不触发动作 */
    ASSERT_EQ_INT(ST_IDLE, fsm_state(&fsm));
    ASSERT_EQ_SIZE(0u, fsm_transition_count(&fsm));

    ASSERT_EQ_INT(FSM_NO_TRANSITION, fsm_peek_next(&fsm, EV_HOLD_DONE, &next));
    ASSERT_EQ_INT(FSM_OK, fsm_peek_next(&fsm, EV_START, NULL)); /* out 可为 NULL */
    return 0;
}

/* ---- reset：中途回到初始态并清零计数 ---- */

static int test_reset_midcycle(void) {
    fsm_t fsm;
    cycle_ctx_t ctx = {0, 0};
    fsm_init(&fsm, kCycleTable, kCycleLen, ST_IDLE, &ctx);

    fsm_dispatch(&fsm, EV_START);
    fsm_dispatch(&fsm, EV_CLAMP_DONE);
    ASSERT_EQ_INT(ST_INJECT, fsm_state(&fsm));
    ASSERT_EQ_SIZE(2u, fsm_transition_count(&fsm));

    ASSERT_EQ_INT(FSM_OK, fsm_reset(&fsm, ST_IDLE));
    ASSERT_EQ_INT(ST_IDLE, fsm_state(&fsm));
    ASSERT_EQ_SIZE(0u, fsm_transition_count(&fsm));   /* 计数清零 */

    /* reset 后仍可正常运行 */
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_START));
    ASSERT_EQ_INT(ST_CLAMP, fsm_state(&fsm));
    return 0;
}

static int test_no_transition_ignored(void) {
    fsm_t fsm;
    cycle_ctx_t ctx = {0, 0};
    fsm_init(&fsm, kCycleTable, kCycleLen, ST_IDLE, &ctx);

    /* 空闲态收到"射胶完成"无意义：应被忽略，状态不变 */
    ASSERT_EQ_INT(FSM_NO_TRANSITION, fsm_dispatch(&fsm, EV_INJECT_DONE));
    ASSERT_EQ_INT(ST_IDLE, fsm_state(&fsm));
    /* 空闲态急停也无规则（表里只对工作态定义了急停） */
    ASSERT_EQ_INT(FSM_NO_TRANSITION, fsm_dispatch(&fsm, EV_ESTOP));
    ASSERT_EQ_INT(ST_IDLE, fsm_state(&fsm));
    return 0;
}

static int test_estop_and_reset(void) {
    fsm_t fsm;
    cycle_ctx_t ctx = {0, 0};
    fsm_init(&fsm, kCycleTable, kCycleLen, ST_IDLE, &ctx);

    fsm_dispatch(&fsm, EV_START);       /* -> CLAMP */
    fsm_dispatch(&fsm, EV_CLAMP_DONE);  /* -> INJECT */
    ASSERT_EQ_INT(ST_INJECT, fsm_state(&fsm));

    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_ESTOP)); /* 射胶中急停 */
    ASSERT_EQ_INT(ST_FAULT, fsm_state(&fsm));
    ASSERT_EQ_INT(1, ctx.estop_count);

    /* 故障态只认复位 */
    ASSERT_EQ_INT(FSM_NO_TRANSITION, fsm_dispatch(&fsm, EV_START));
    ASSERT_EQ_INT(ST_FAULT, fsm_state(&fsm));
    ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, EV_RESET));
    ASSERT_EQ_INT(ST_IDLE, fsm_state(&fsm));
    return 0;
}

static int test_two_cycles(void) {
    fsm_t fsm;
    cycle_ctx_t ctx = {0, 0};
    fsm_init(&fsm, kCycleTable, kCycleLen, ST_IDLE, &ctx);

    const int seq[] = {
        EV_START, EV_CLAMP_DONE, EV_INJECT_DONE, EV_HOLD_DONE,
        EV_CHARGE_DONE, EV_COOL_DONE, EV_OPEN_DONE, EV_EJECT_DONE
    };
    for (int rep = 0; rep < 2; rep++) {
        for (size_t i = 0; i < sizeof(seq) / sizeof(seq[0]); i++) {
            ASSERT_EQ_INT(FSM_OK, fsm_dispatch(&fsm, seq[i]));
        }
        ASSERT_EQ_INT(ST_IDLE, fsm_state(&fsm));
    }
    ASSERT_EQ_INT(2, ctx.cycle_done);
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_basic);
    RUN_TEST(test_init_null);
    RUN_TEST(test_full_cycle);
    RUN_TEST(test_peek_and_can);
    RUN_TEST(test_reset_midcycle);
    RUN_TEST(test_no_transition_ignored);
    RUN_TEST(test_estop_and_reset);
    RUN_TEST(test_two_cycles);
    TEST_END();
}
