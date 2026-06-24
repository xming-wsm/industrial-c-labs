/**
 * @file fsm.h
 * @brief Lab 5 - 表驱动有限状态机（注塑周期）API 声明。
 *
 * 工业背景
 * --------
 * 注塑机的一个生产周期是严格的状态序列：
 *     空闲 → 合模 → 射胶 → 保压 → 储料 → 冷却 → 开模 → 顶出 → (回到空闲)
 * 每个阶段在某个"完成事件"到来时推进到下一阶段；任何阶段收到"急停"
 * 都要立刻跳到故障态。把这些规则写成一堆 if/else 既难读又易错。
 *
 * 工业界标准做法是**表驱动状态机**：把"在状态 X 收到事件 E 就转到状态 Y
 * 并执行动作 A"这一条条规则塞进一张**转移表**，引擎只负责查表 + 跳转。
 * 加一条规则就是加一行数据，逻辑与数据分离，清晰且易维护。
 *
 * 设计约束
 * --------
 *   1. 不使用 malloc/free：转移表由调用者提供（通常是 const 静态数组）。
 *   2. 函数对 NULL 参数安全。
 *   3. 引擎与具体状态/事件解耦：引擎只认下标，业务表自己定义含义。
 */
#ifndef LAB05_FSM_H
#define LAB05_FSM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** dispatch 的返回状态。 */
typedef enum {
    FSM_OK = 0,             /**< 命中转移并完成跳转           */
    FSM_ERR_NULL,           /**< 传入了 NULL 指针             */
    FSM_NO_TRANSITION       /**< 当前状态对该事件无转移（忽略，状态不变） */
} fsm_status_t;

/**
 * 转移动作回调：发生转移时被调用（可为 NULL 表示无动作）。
 * @param ctx 用户上下文（fsm_init 时传入），动作可借此读写业务数据。
 */
typedef void (*fsm_action_fn)(void *ctx);

/**
 * 一条转移规则：在状态 from 收到事件 event，跳到 to，并执行 action。
 * 状态与事件都用整型编号表示，具体含义由业务方（你的转移表）定义。
 */
typedef struct {
    int            from;    /**< 源状态编号       */
    int            event;   /**< 触发事件编号     */
    int            to;      /**< 目标状态编号     */
    fsm_action_fn  action;  /**< 转移动作（可 NULL） */
} fsm_transition_t;

/**
 * 状态机控制块。
 * 字段公开仅为方便测试与教学，正常使用请只通过 API 访问。
 */
typedef struct {
    const fsm_transition_t *table;      /**< 转移表（调用者提供）       */
    size_t                  table_len;  /**< 转移表行数                 */
    int                     state;      /**< 当前状态编号               */
    void                   *ctx;        /**< 传给 action 的用户上下文   */
    size_t                  transitions;/**< 成功转移累计次数（统计用） */
} fsm_t;

/**
 * 初始化状态机。
 *
 * @param fsm        控制块。
 * @param table      转移表（至少 table_len 行），可为 const 静态数组。
 * @param table_len  转移表行数，必须 > 0。
 * @param initial    初始状态编号。
 * @param ctx        用户上下文（传给 action），可为 NULL。
 * @return FSM_OK；fsm/table 为 NULL 或 table_len==0 -> FSM_ERR_NULL。
 */
fsm_status_t fsm_init(fsm_t *fsm,
                      const fsm_transition_t *table, size_t table_len,
                      int initial, void *ctx);

/** @return 当前状态编号；fsm 为 NULL 时返回 -1。 */
int fsm_state(const fsm_t *fsm);

/**
 * 把状态机重置到指定初始状态（保留已绑定的转移表与 ctx），
 * 同时把转移计数清零。常用于一轮生产结束后复用同一实例。
 * @return FSM_OK；fsm 为 NULL -> FSM_ERR_NULL。
 */
fsm_status_t fsm_reset(fsm_t *fsm, int initial);

/**
 * 派发一个事件：在转移表里查找 (当前状态, event) 匹配的行。
 *   - 命中：切换到目标状态，若 action 非 NULL 则调用 action(ctx)，
 *           转移计数 +1，返回 FSM_OK。
 *   - 未命中：状态不变，返回 FSM_NO_TRANSITION。
 *
 * @return FSM_OK / FSM_NO_TRANSITION；fsm 为 NULL -> FSM_ERR_NULL。
 */
fsm_status_t fsm_dispatch(fsm_t *fsm, int event);

/**
 * 查询当前状态下该事件是否存在可用转移（不改变状态、不触发动作）。
 * @return true 存在转移；false 不存在（或 fsm 为 NULL）。
 */
bool fsm_can_dispatch(const fsm_t *fsm, int event);

/**
 * 预览：若在当前状态派发 event，会跳到哪个状态（不真正跳转、不触发动作）。
 * @param out_next 命中时写入目标状态编号（可为 NULL）。
 * @return FSM_OK 命中；FSM_NO_TRANSITION 无转移；fsm 为 NULL -> FSM_ERR_NULL。
 */
fsm_status_t fsm_peek_next(const fsm_t *fsm, int event, int *out_next);

/** @return 自 init/reset 以来成功转移的累计次数；fsm 为 NULL 时返回 0。 */
size_t fsm_transition_count(const fsm_t *fsm);

#ifdef __cplusplus
}
#endif

#endif /* LAB05_FSM_H */
