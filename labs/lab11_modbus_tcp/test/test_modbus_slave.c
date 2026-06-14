/**
 * @file test_modbus_slave.c
 * @brief Lab 11 测试套件。无需修改本文件；实现 modbus_slave.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "modbus_slave.h"

static void wr16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xFF);
}
static uint16_t rd16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

/* 构造一帧 Modbus TCP 请求 ADU。返回长度（恒为 12）。 */
static size_t build_req(uint8_t *buf, uint16_t tid, uint8_t unit,
                        uint8_t fc, uint16_t a, uint16_t b) {
    wr16(buf, tid);       /* 事务ID   */
    wr16(buf + 2, 0);     /* 协议ID=0 */
    wr16(buf + 4, 6);     /* 长度=单元ID(1)+PDU(5) */
    buf[6] = unit;        /* 单元ID   */
    buf[7] = fc;          /* 功能码   */
    wr16(buf + 8, a);     /* 地址/起始 */
    wr16(buf + 10, b);    /* 值/数量   */
    return 12;
}

static int test_init_null(void) {
    uint16_t regs[8];
    modbus_slave_t mb;
    ASSERT_EQ_INT(-1, mb_init(NULL, regs, 8));
    ASSERT_EQ_INT(-1, mb_init(&mb, NULL, 8));
    ASSERT_EQ_INT(-1, mb_init(&mb, regs, 0));

    ASSERT_EQ_INT(0, mb_init(&mb, regs, 8));
    /* 初始化后寄存器应清零 */
    uint16_t v = 0xFFFF;
    ASSERT_EQ_INT(0, mb_get_register(&mb, 0, &v));
    ASSERT_EQ_INT(0, (int)v);

    ASSERT_EQ_INT(-1, mb_set_register(&mb, 8, 1));   /* 越界 */
    ASSERT_EQ_INT(-1, mb_get_register(&mb, 8, &v));  /* 越界 */
    return 0;
}

static int test_read_holding(void) {
    uint16_t regs[16];
    modbus_slave_t mb;
    mb_init(&mb, regs, 16);
    mb_set_register(&mb, 0, 0x1111);
    mb_set_register(&mb, 1, 0x2222);
    mb_set_register(&mb, 2, 0x3333);

    uint8_t req[16];
    size_t rl = build_req(req, 0xABCD, 1, MB_FC_READ_HOLDING, 0, 3);

    uint8_t resp[64];
    int n = mb_process(&mb, req, rl, resp, sizeof(resp));
    ASSERT_EQ_INT(15, n);                 /* 7 MBAP + PDU(1+1+6) */

    ASSERT_EQ_INT(0xABCD, (int)rd16(resp));     /* 回显事务ID */
    ASSERT_EQ_INT(0, (int)rd16(resp + 2));      /* 协议ID=0   */
    ASSERT_EQ_INT(9, (int)rd16(resp + 4));      /* 长度=1+8   */
    ASSERT_EQ_INT(1, resp[6]);                  /* 单元ID     */
    ASSERT_EQ_INT(MB_FC_READ_HOLDING, resp[7]);
    ASSERT_EQ_INT(6, resp[8]);                  /* 字节数=2*3 */
    ASSERT_EQ_INT(0x1111, (int)rd16(resp + 9));
    ASSERT_EQ_INT(0x2222, (int)rd16(resp + 11));
    ASSERT_EQ_INT(0x3333, (int)rd16(resp + 13));
    return 0;
}

static int test_write_single(void) {
    uint16_t regs[16];
    modbus_slave_t mb;
    mb_init(&mb, regs, 16);

    uint8_t req[16];
    size_t rl = build_req(req, 0x0007, 1, MB_FC_WRITE_SINGLE, 5, 0xBEEF);

    uint8_t resp[64];
    int n = mb_process(&mb, req, rl, resp, sizeof(resp));
    ASSERT_EQ_INT(12, n);                 /* 7 + PDU(5) */
    ASSERT_EQ_INT(0x0007, (int)rd16(resp));
    ASSERT_EQ_INT(6, (int)rd16(resp + 4)); /* 长度=1+5 */
    ASSERT_EQ_INT(MB_FC_WRITE_SINGLE, resp[7]);
    ASSERT_EQ_INT(5, (int)rd16(resp + 8));      /* 回显地址 */
    ASSERT_EQ_INT(0xBEEF, (int)rd16(resp + 10));/* 回显值   */

    /* 寄存器确实被写入 */
    uint16_t v = 0;
    ASSERT_EQ_INT(0, mb_get_register(&mb, 5, &v));
    ASSERT_EQ_INT(0xBEEF, (int)v);
    return 0;
}

static int test_exception_illegal_address(void) {
    uint16_t regs[16];
    modbus_slave_t mb;
    mb_init(&mb, regs, 16);

    uint8_t req[16];
    /* 读 14..18 越界（14+5 > 16） */
    size_t rl = build_req(req, 1, 1, MB_FC_READ_HOLDING, 14, 5);

    uint8_t resp[64];
    int n = mb_process(&mb, req, rl, resp, sizeof(resp));
    ASSERT_EQ_INT(9, n);                          /* 7 + 异常PDU(2) */
    ASSERT_EQ_INT(MB_FC_READ_HOLDING | 0x80, resp[7]);  /* 0x83 */
    ASSERT_EQ_INT(MB_EX_ILLEGAL_ADDRESS, resp[8]);
    return 0;
}

static int test_exception_illegal_value(void) {
    uint16_t regs[16];
    modbus_slave_t mb;
    mb_init(&mb, regs, 16);

    uint8_t req[16];
    uint8_t resp[64];

    /* 数量 0 非法 */
    size_t rl = build_req(req, 1, 1, MB_FC_READ_HOLDING, 0, 0);
    int n = mb_process(&mb, req, rl, resp, sizeof(resp));
    ASSERT_EQ_INT(9, n);
    ASSERT_EQ_INT(0x83, resp[7]);
    ASSERT_EQ_INT(MB_EX_ILLEGAL_VALUE, resp[8]);

    /* 数量 126 超过 125 非法 */
    rl = build_req(req, 1, 1, MB_FC_READ_HOLDING, 0, 126);
    n = mb_process(&mb, req, rl, resp, sizeof(resp));
    ASSERT_EQ_INT(9, n);
    ASSERT_EQ_INT(MB_EX_ILLEGAL_VALUE, resp[8]);
    return 0;
}

static int test_exception_illegal_function(void) {
    uint16_t regs[16];
    modbus_slave_t mb;
    mb_init(&mb, regs, 16);

    uint8_t req[16];
    size_t rl = build_req(req, 1, 1, 0x10 /* 不支持 */, 0, 1);

    uint8_t resp[64];
    int n = mb_process(&mb, req, rl, resp, sizeof(resp));
    ASSERT_EQ_INT(9, n);
    ASSERT_EQ_INT(0x90, resp[7]);                 /* 0x10 | 0x80 */
    ASSERT_EQ_INT(MB_EX_ILLEGAL_FUNCTION, resp[8]);
    return 0;
}

static int test_write_illegal_address(void) {
    uint16_t regs[16];
    modbus_slave_t mb;
    mb_init(&mb, regs, 16);

    uint8_t req[16];
    size_t rl = build_req(req, 1, 1, MB_FC_WRITE_SINGLE, 20, 0x1234);

    uint8_t resp[64];
    int n = mb_process(&mb, req, rl, resp, sizeof(resp));
    ASSERT_EQ_INT(9, n);
    ASSERT_EQ_INT(0x86, resp[7]);                 /* 0x06 | 0x80 */
    ASSERT_EQ_INT(MB_EX_ILLEGAL_ADDRESS, resp[8]);
    return 0;
}

static int test_malformed(void) {
    uint16_t regs[16];
    modbus_slave_t mb;
    mb_init(&mb, regs, 16);

    uint8_t req[16];
    uint8_t resp[64];

    /* 太短无法解析 */
    ASSERT_EQ_INT(-1, mb_process(&mb, req, 5, resp, sizeof(resp)));

    /* 协议ID 非 0 */
    size_t rl = build_req(req, 1, 1, MB_FC_READ_HOLDING, 0, 1);
    req[2] = 0; req[3] = 1;   /* 协议ID = 1 */
    ASSERT_EQ_INT(-1, mb_process(&mb, req, rl, resp, sizeof(resp)));

    /* 应答缓冲太小 */
    rl = build_req(req, 1, 1, MB_FC_READ_HOLDING, 0, 3);
    ASSERT_EQ_INT(-1, mb_process(&mb, req, rl, resp, 4));

    /* NULL 安全 */
    ASSERT_EQ_INT(-1, mb_process(NULL, req, rl, resp, sizeof(resp)));
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_init_null);
    RUN_TEST(test_read_holding);
    RUN_TEST(test_write_single);
    RUN_TEST(test_exception_illegal_address);
    RUN_TEST(test_exception_illegal_value);
    RUN_TEST(test_exception_illegal_function);
    RUN_TEST(test_write_illegal_address);
    RUN_TEST(test_malformed);
    TEST_END();
}
