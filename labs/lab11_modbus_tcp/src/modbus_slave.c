/**
 * @file modbus_slave.c
 * @brief Lab 11 - Modbus TCP 从站（待你完成）
 *
 * 详见 docs/lab11_modbus_tcp.md；测试：xmake lab11 test
 */
#include "modbus_slave.h"

int mb_init(modbus_slave_t *mb, uint16_t *regs, size_t nregs) {
    (void)mb;
    (void)regs;
    (void)nregs;
    return -1;
}

int mb_set_register(modbus_slave_t *mb, uint16_t addr, uint16_t value) {
    (void)mb;
    (void)addr;
    (void)value;
    return -1;
}

int mb_get_register(const modbus_slave_t *mb, uint16_t addr, uint16_t *out) {
    (void)mb;
    (void)addr;
    (void)out;
    return -1;
}

int mb_process(modbus_slave_t *mb,
               const uint8_t *req, size_t req_len,
               uint8_t *resp, size_t resp_cap) {
    (void)mb;
    (void)req;
    (void)req_len;
    (void)resp;
    (void)resp_cap;
    return -1;
}
