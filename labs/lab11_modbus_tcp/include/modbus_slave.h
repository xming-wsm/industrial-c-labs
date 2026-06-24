/**
 * @file modbus_slave.h
 * @brief Lab 11 - Modbus TCP 从站（slave）模拟 API 声明。
 *
 * 工业背景
 * --------
 * Modbus 是工业领域最普及的协议之一，注塑机、温控表、伺服、PLC 几乎都支持。
 * Modbus TCP 把 Modbus 报文封进 TCP，由"主站（master，如上位机/SCADA）"
 * 发请求、"从站（slave，如本注塑机）"回应答。从站维护一组**保持寄存器
 * （holding registers）**，主站通过寄存器地址读写注塑机参数与状态。
 *
 * 本关实现一个**纯函数式**的从站请求处理器：输入一帧请求 ADU，输出一帧
 * 应答 ADU。不涉及网络（网络部分是 Lab9），便于确定性单元测试。把它接到
 * Lab9 的 TCP 收发上，就是一个能跑的 Modbus TCP 从站。
 *
 * 报文结构（Modbus TCP ADU）
 * --------------------------
 *   MBAP 头(7B): [事务ID hi,lo][协议ID=0 hi,lo][长度 hi,lo][单元ID]
 *   PDU        : [功能码][数据...]
 * 支持的功能码：
 *   0x03 读保持寄存器：请求 [FC][起始地址 hi,lo][数量 hi,lo]
 *                      应答 [FC][字节数][寄存器值 hi,lo ...]
 *   0x06 写单个寄存器：请求 [FC][地址 hi,lo][值 hi,lo]
 *                      应答 原样回显 [FC][地址][值]
 *   0x10 写多个寄存器：请求 [FC][起始地址 hi,lo][数量 hi,lo][字节数][值 hi,lo ...]
 *                      应答 [FC][起始地址 hi,lo][数量 hi,lo]
 * 异常应答：[FC|0x80][异常码]
 */
#ifndef LAB11_MODBUS_SLAVE_H
#define LAB11_MODBUS_SLAVE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 功能码。 */
#define MB_FC_READ_HOLDING   0x03
#define MB_FC_WRITE_SINGLE   0x06
#define MB_FC_WRITE_MULTIPLE 0x10

/** 异常码。 */
#define MB_EX_ILLEGAL_FUNCTION  0x01
#define MB_EX_ILLEGAL_ADDRESS   0x02
#define MB_EX_ILLEGAL_VALUE     0x03

/** 单次读保持寄存器的最大数量（Modbus 规范）。 */
#define MB_MAX_READ_REGS  125

/** 单次写多个寄存器的最大数量（Modbus 规范，0x10）。 */
#define MB_MAX_WRITE_REGS 123

/** 从站控制块：持有一组保持寄存器，并统计请求/异常计数。 */
typedef struct {
    uint16_t *regs;             /**< 保持寄存器数组（调用者提供）       */
    size_t    nregs;            /**< 寄存器个数                         */
    uint32_t  request_count;    /**< 成功产生应答的请求数（含异常应答） */
    uint32_t  exception_count;  /**< 其中产生异常应答的次数             */
} modbus_slave_t;

/**
 * 初始化从站，绑定寄存器表（全部清零）。
 * @return 0 成功；-1（mb/regs 为 NULL 或 nregs==0）。
 */
int mb_init(modbus_slave_t *mb, uint16_t *regs, size_t nregs);

/** 直接写一个寄存器（测试/本地用）。@return 0；地址越界或 mb NULL -> -1。 */
int mb_set_register(modbus_slave_t *mb, uint16_t addr, uint16_t value);

/** 直接读一个寄存器（测试/本地用）。@return 0；地址越界或 mb/out NULL -> -1。 */
int mb_get_register(const modbus_slave_t *mb, uint16_t addr, uint16_t *out);

/** @return 寄存器总数；mb 为 NULL 时返回 0。 */
size_t mb_register_count(const modbus_slave_t *mb);

/** @return 累计产生应答的请求数（含异常应答）；mb 为 NULL 时返回 0。 */
uint32_t mb_request_count(const modbus_slave_t *mb);

/** @return 累计产生异常应答的次数；mb 为 NULL 时返回 0。 */
uint32_t mb_exception_count(const modbus_slave_t *mb);

/**
 * 处理一帧请求 ADU，生成一帧应答 ADU。
 *
 * @param mb        从站。
 * @param req       请求 ADU 字节。
 * @param req_len   请求长度。
 * @param resp      应答输出缓冲。
 * @param resp_cap  应答缓冲容量。
 * @return 应答 ADU 的字节数；
 *         报文无法解析（长度不足 / 协议ID 非 0 / 应答缓冲不够）时返回 -1（不产生应答）。
 *         注意：地址/数量/功能码不合法属于"正常的异常应答"，返回的是异常帧长度（> 0）。
 */
int mb_process(modbus_slave_t *mb,
               const uint8_t *req, size_t req_len,
               uint8_t *resp, size_t resp_cap);

#ifdef __cplusplus
}
#endif

#endif /* LAB11_MODBUS_SLAVE_H */
