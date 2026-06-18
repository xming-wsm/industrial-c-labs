/**
 * @file frame_parser.c
 * @brief Lab 10 - 协议帧解析 + CRC16（待你完成）
 *
 * 详见 docs/lab10_frame_parser.md；测试：xmake lab10 test
 */
#include "frame_parser.h"

uint16_t crc16_modbus(const uint8_t *data, size_t len) {
    (void)data;
    (void)len;
    return 0xFFFF;
}

int fp_init(frame_parser_t *fp, fp_frame_cb cb, void *ctx) {
    (void)fp;
    (void)cb;
    (void)ctx;
    return -1;
}

void fp_reset(frame_parser_t *fp) {
    (void)fp;
}

size_t fp_feed(frame_parser_t *fp, const uint8_t *data, size_t len) {
    (void)fp;
    (void)data;
    (void)len;
    return 0;
}

uint32_t fp_good_count(const frame_parser_t *fp) {
    (void)fp;
    return 0;
}

uint32_t fp_crc_error_count(const frame_parser_t *fp) {
    (void)fp;
    return 0;
}

uint32_t fp_frame_error_count(const frame_parser_t *fp) {
    (void)fp;
    return 0;
}

int fp_build(uint8_t *out, size_t out_cap, const uint8_t *payload, uint8_t len) {
    (void)out;
    (void)out_cap;
    (void)payload;
    (void)len;
    return -1;
}
