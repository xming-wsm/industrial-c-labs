/**
 * @file alarm_list.h
 * @brief Lab 3 - 链表 + 静态内存池（报警链表）API 声明。
 *
 * 工业背景
 * --------
 * 注塑机运行时会产生数量不定的报警（超温、超压、缺料、伺服故障……）。
 * 报警条目需要：随时新增、随时按报警码清除、可遍历当前所有未恢复报警。
 * 这是典型的"链表"需求——但嵌入式 / 实时系统**禁止运行时 malloc**
 * （内存碎片、耗时不确定、可能失败）。
 *
 * 解决办法：**静态内存池 + 自由链表（free list）**。
 *   - 上电时一次性拿到一块节点数组（调用者提供）；
 *   - 把所有节点串成"空闲链表"；
 *   - 分配 = 从空闲链表头摘一个（O(1)）；
 *   - 释放 = 还回空闲链表头（O(1)）。
 * 这样既有链表的灵活，又有"内存可控、耗时可预测"的实时特性。
 *
 * 设计约束
 * --------
 *   1. 不使用 malloc/free：节点数组由调用者提供。
 *   2. 函数对 NULL 参数安全。
 *   3. 分配 / 释放都是 O(1)（自由链表）。
 */
#ifndef LAB03_ALARM_LIST_H
#define LAB03_ALARM_LIST_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 操作返回状态码。 */
typedef enum {
    LL_OK = 0,          /**< 成功                       */
    LL_ERR_NULL,        /**< 传入了 NULL 指针           */
    LL_ERR_FULL,        /**< 内存池耗尽，无法分配新节点 */
    LL_ERR_NOT_FOUND    /**< 未找到指定报警码           */
} ll_status_t;

/** 链表节点 = 一条报警。next 串起业务链表或空闲链表。 */
typedef struct ll_node {
    uint16_t        code;   /**< 报警码（key）          */
    int32_t         value;  /**< 附带数据（如实测值）   */
    struct ll_node *next;   /**< 下一节点               */
} ll_node_t;

/**
 * 报警链表控制块：同时管理"内存池"与"业务链表"。
 * 字段公开仅为方便测试与教学，正常使用请只通过 API 访问。
 */
typedef struct {
    ll_node_t *pool;        /**< 调用者提供的节点数组         */
    size_t     capacity;    /**< 池中节点总数                 */
    ll_node_t *free_head;   /**< 空闲链表头（可分配的节点）   */
    ll_node_t *list_head;   /**< 业务链表头（当前报警）       */
    size_t     used;        /**< 已分配（在用）的节点数       */
} alarm_list_t;

/**
 * 初始化：绑定节点数组，并把所有节点串成空闲链表。
 *
 * @param al        控制块指针。
 * @param storage   ll_node_t 数组（至少 capacity 个）。
 * @param capacity  节点总数，必须 > 0。
 * @return LL_OK；LL_ERR_NULL（al/storage 为 NULL）；LL_ERR_FULL 不会在此返回；
 *         capacity==0 也返回 LL_ERR_NULL（视为非法参数）。
 */
ll_status_t al_init(alarm_list_t *al, ll_node_t *storage, size_t capacity);

/** 清空业务链表，把所有节点还回空闲链表。al 为 NULL 时安全返回。 */
void al_reset(alarm_list_t *al);

/** @return 当前在用（已分配）的节点数；al 为 NULL 时返回 0。 */
size_t al_count(const alarm_list_t *al);

/** @return 池容量；al 为 NULL 时返回 0。 */
size_t al_capacity(const alarm_list_t *al);

/** @return 池是否耗尽（used == capacity）；al 为 NULL 时返回 false。 */
bool al_is_full(const alarm_list_t *al);

/* ---- 内存池底层（O(1) 自由链表） ---- */

/**
 * 从池中分配一个节点（不挂入业务链表）。
 * @return 节点指针；池耗尽或 al 为 NULL 时返回 NULL。
 *         分配出的节点 next 不保证清零，调用方自行设置。
 */
ll_node_t *al_node_alloc(alarm_list_t *al);

/**
 * 把节点还回池（不负责从业务链表摘除）。
 * al / node 为 NULL 时安全返回（什么也不做）。
 */
void al_node_free(alarm_list_t *al, ll_node_t *node);

/* ---- 业务链表操作（基于内存池） ---- */

/**
 * 新增一条报警，追加到业务链表尾部。
 * @return LL_OK；LL_ERR_NULL（al 为 NULL）；LL_ERR_FULL（池耗尽）。
 */
ll_status_t al_add(alarm_list_t *al, uint16_t code, int32_t value);

/**
 * 按报警码查找。
 * @param out_value 命中时把 value 写入 *out_value（可为 NULL）。
 * @return true 找到；false 未找到（或 al 为 NULL）。
 */
bool al_find(const alarm_list_t *al, uint16_t code, int32_t *out_value);

/**
 * 移除"第一个"匹配该报警码的节点，并把节点还回池。
 * @return LL_OK；LL_ERR_NULL（al 为 NULL）；LL_ERR_NOT_FOUND（无此码）。
 */
ll_status_t al_remove(alarm_list_t *al, uint16_t code);

#ifdef __cplusplus
}
#endif

#endif /* LAB03_ALARM_LIST_H */
