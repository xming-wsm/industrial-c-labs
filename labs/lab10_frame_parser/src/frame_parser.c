/**
 * @file frame_parser.c
 * @brief Lab 10 - 协议帧解析 + CRC16 实现（待你完成）。
 *
 * 你的任务：把每个函数里的 TODO 实现掉，使 test/test_frame_parser.c 全部通过。
 *
 * 实现要点（强烈建议先读 docs/lab10_frame_parser.md）：
 *   - crc16_modbus：crc=0xFFFF；对每个字节 crc ^= b；
 *       循环 8 次：if (crc & 1) crc = (crc >> 1) ^ 0xA001; else crc >>= 1;
 *   - fp_feed：逐字节跑状态机：
 *       WAIT_STX  : b==FP_STX -> WAIT_LEN
 *       WAIT_LEN  : len=b; idx=0; 若 len>FP_MAX_PAYLOAD -> frame_errors++ 回 WAIT_STX；
 *                   否则 (len==0 ? WAIT_CRC_LO : WAIT_PAYLOAD)
 *       WAIT_PAYLOAD: payload[idx++]=b; 收满 len -> WAIT_CRC_LO
 *       WAIT_CRC_LO : crc_rx = b
 *       WAIT_CRC_HI : crc_rx |= (b << 8)
 *       WAIT_ETX    : 若 b==FP_ETX 则比对 crc16_modbus(payload,len) 与 crc_rx：
 *                       相等 -> good_frames++，cb(payload,len,ctx)，本次交付计数+1；
 *                       不等 -> crc_errors++；
 *                     b!=ETX -> frame_errors++；最后都回 WAIT_STX。
 *   - fp_build：out = [STX][len][payload...][crc_lo][crc_hi][ETX]，返回 len+5。
 */
#include "frame_parser.h"

uint16_t crc16_modbus(const uint8_t *data, size_t len) {
    /* TODO: 见文件头。data 为 NULL 或 len==0 时返回 0xFFFF。 */
    (void)data;
    (void)len;
    return 0xFFFF;
}

int fp_init(frame_parser_t *fp, fp_frame_cb cb, void *ctx) {
    /* TODO:
     *   fp 为 NULL -> -1；
     *   绑定 cb/ctx；state=FP_WAIT_STX；len/idx/crc_rx=0；
     *   good_frames/crc_errors/frame_errors=0 -> 返回 0。
     */
    (void)fp;
    (void)cb;
    (void)ctx;
    return -1; /* 占位：实现前测试应当失败 */
}

void fp_reset(frame_parser_t *fp) {
    /* TODO: state=FP_WAIT_STX；len/idx/crc_rx=0；三个计数清零（保留 cb/ctx）。 */
    (void)fp;
}

size_t fp_feed(frame_parser_t *fp, const uint8_t *data, size_t len) {
    /* TODO: 见文件头"实现要点"，逐字节驱动状态机，返回本次交付的完整帧数。 */
    (void)fp;
    (void)data;
    (void)len;
    return 0;
}

uint32_t fp_good_count(const frame_parser_t *fp) {
    /* TODO: NULL 返回 0；否则返回 good_frames。 */
    (void)fp;
    return 0;
}

uint32_t fp_crc_error_count(const frame_parser_t *fp) {
    /* TODO: NULL 返回 0；否则返回 crc_errors。 */
    (void)fp;
    return 0;
}

uint32_t fp_frame_error_count(const frame_parser_t *fp) {
    /* TODO: NULL 返回 0；否则返回 frame_errors。 */
    (void)fp;
    return 0;
}

int fp_build(uint8_t *out, size_t out_cap, const uint8_t *payload, uint8_t len) {
    /* TODO:
     *   out 为 NULL，或 len>FP_MAX_PAYLOAD，或 (len>0 且 payload 为 NULL)，
     *   或 out_cap < (size_t)len + 5 -> 返回 -1；
     *   写入 [STX][len][payload...][crc_lo][crc_hi][ETX]，
     *   crc = crc16_modbus(payload, len)，小端；返回 len + 5。
     */
    (void)out;
    (void)out_cap;
    (void)payload;
    (void)len;
    return -1;
}
