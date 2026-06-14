/**
 * @file modbus_slave.c
 * @brief Lab 11 - Modbus TCP 从站请求处理实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_modbus_slave.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab11_modbus_tcp.md）：
 *   - 大端读写：寄存器/地址/数量在报文里都是高字节在前（big-endian）。
 *       rd16(p) = (p[0]<<8) | p[1];  wr16(p,v){ p[0]=v>>8; p[1]=v&0xFF; }
 *   - MBAP 头 7 字节：事务ID(2) 协议ID(2,=0) 长度(2) 单元ID(1)，PDU 从第 7 字节起。
 *   - 应答 MBAP：回显事务ID 与单元ID；协议ID=0；长度 = 1(单元ID) + PDU字节数。
 *   - FC 0x03 读保持寄存器：
 *       请求 PDU = [0x03][起始地址 hi,lo][数量 hi,lo]（请求总长 12 字节）
 *       数量必须 1..125（否则异常 0x03）；起始地址+数量 必须 <= nregs（否则异常 0x02）；
 *       应答 PDU = [0x03][字节数=2*数量][寄存器值 hi,lo ...]。
 *   - FC 0x06 写单个寄存器：
 *       请求 PDU = [0x06][地址 hi,lo][值 hi,lo]（请求总长 12 字节）
 *       地址 >= nregs 异常 0x02；否则写入并原样回显请求 PDU。
 *   - 其它功能码：异常 0x01。
 *   - 异常应答 PDU = [FC|0x80][异常码]。
 *   - 报文长度不足以解析、协议ID 非 0、或应答缓冲不够 -> 返回 -1（不产生应答）。
 */
#include "modbus_slave.h"

int mb_init(modbus_slave_t *mb, uint16_t *regs, size_t nregs) {
    /* TODO:
     *   mb/regs 为 NULL 或 nregs==0 -> -1；
     *   绑定 regs/nregs，并把所有寄存器清零 -> 返回 0。
     */
    (void)mb;
    (void)regs;
    (void)nregs;
    return -1; /* 占位：实现前测试应当失败 */
}

int mb_set_register(modbus_slave_t *mb, uint16_t addr, uint16_t value) {
    /* TODO: mb 为 NULL 或 addr>=nregs -> -1；否则 regs[addr]=value，返回 0。 */
    (void)mb;
    (void)addr;
    (void)value;
    return -1;
}

int mb_get_register(const modbus_slave_t *mb, uint16_t addr, uint16_t *out) {
    /* TODO: mb/out 为 NULL 或 addr>=nregs -> -1；否则 *out=regs[addr]，返回 0。 */
    (void)mb;
    (void)addr;
    (void)out;
    return -1;
}

int mb_process(modbus_slave_t *mb,
               const uint8_t *req, size_t req_len,
               uint8_t *resp, size_t resp_cap) {
    /* TODO: 见文件头"实现要点"。建议步骤：
     *   1. 基本校验：mb/req/resp 非 NULL；req_len >= 8（7 MBAP + 至少 1 FC）；
     *      协议ID（req[2],req[3]）必须为 0；否则 -> -1。
     *   2. 解析 MBAP，准备写应答头（回显事务ID/单元ID，协议ID=0）。
     *   3. 取功能码 req[7]，按 0x03 / 0x06 / 其它 分别构造 PDU（或异常 PDU）。
     *   4. 回填 MBAP 长度字段 = 1 + PDU 长度；
     *      确保 7 + PDU 长度 <= resp_cap，否则 -> -1。
     *   5. 返回应答总字节数 = 7 + PDU 长度。
     */
    (void)mb;
    (void)req;
    (void)req_len;
    (void)resp;
    (void)resp_cap;
    return -1;
}
