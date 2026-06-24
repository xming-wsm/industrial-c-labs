/**
 * @file test_frame_parser.c
 * @brief Lab 10 测试套件。无需修改本文件；实现 frame_parser.c 使其全部通过即可。
 */
#include "test_framework.h"
#include "frame_parser.h"
#include <string.h>

/* 帧交付回调：记录最近一帧负载与调用次数。 */
typedef struct {
    uint8_t buf[FP_MAX_PAYLOAD];
    uint8_t len;
    int     calls;
} cap_t;

static void on_frame(const uint8_t *payload, uint8_t len, void *ctx) {
    cap_t *c = (cap_t *)ctx;
    if (len <= FP_MAX_PAYLOAD) memcpy(c->buf, payload, len);
    c->len = len;
    c->calls++;
}

/* ---- CRC16/MODBUS 已知向量 ---- */

static int test_crc_known_vector(void) {
    /* CRC-16/MODBUS("123456789") == 0x4B37（业界标准校验值）。 */
    ASSERT_EQ_INT(0x4B37, (int)crc16_modbus((const uint8_t *)"123456789", 9));
    /* NULL / 空 -> 初值 0xFFFF */
    ASSERT_EQ_INT(0xFFFF, (int)crc16_modbus(NULL, 0));
    ASSERT_EQ_INT(0xFFFF, (int)crc16_modbus((const uint8_t *)"x", 0));
    return 0;
}

/* ---- build + 解析一整帧 ---- */

static int test_build_and_parse(void) {
    const uint8_t payload[4] = {'A', 'B', 'C', 'D'};
    uint8_t frame[16];
    int flen = fp_build(frame, sizeof(frame), payload, 4);
    ASSERT_EQ_INT(9, flen);                 /* 4 + 5 */
    ASSERT_EQ_INT(FP_STX, frame[0]);
    ASSERT_EQ_INT(4, frame[1]);
    ASSERT_EQ_INT(FP_ETX, frame[8]);

    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    ASSERT_EQ_INT(0, fp_init(&fp, on_frame, &cap));

    ASSERT_EQ_SIZE(1u, fp_feed(&fp, frame, (size_t)flen));
    ASSERT_EQ_INT(1, cap.calls);
    ASSERT_EQ_INT(4, cap.len);
    ASSERT_EQ_MEM(payload, cap.buf, 4);
    ASSERT_EQ_INT(1, (int)fp_good_count(&fp));
    ASSERT_EQ_INT(0, (int)fp_crc_error_count(&fp));
    return 0;
}

/* ---- 分包：逐字节喂入仍能拼出一帧 ---- */

static int test_byte_by_byte(void) {
    const uint8_t payload[3] = {0x11, 0x22, 0x33};
    uint8_t frame[16];
    int flen = fp_build(frame, sizeof(frame), payload, 3);
    ASSERT_TRUE(flen > 0);

    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    fp_init(&fp, on_frame, &cap);

    size_t delivered = 0;
    for (int i = 0; i < flen; i++) {
        delivered += fp_feed(&fp, &frame[i], 1);
    }
    ASSERT_EQ_SIZE(1u, delivered);
    ASSERT_EQ_INT(1, cap.calls);
    ASSERT_EQ_MEM(payload, cap.buf, 3);
    return 0;
}

/* ---- 粘包 + 前导垃圾：一次喂入两帧 ---- */

static int test_two_frames_with_garbage(void) {
    const uint8_t p1[2] = {0xAA, 0xBB};
    const uint8_t p2[5] = {1, 2, 3, 4, 5};
    uint8_t stream[64];
    size_t off = 0;

    /* 前导垃圾（非 STX 应被忽略） */
    stream[off++] = 0xFF;
    stream[off++] = 0x00;

    int n1 = fp_build(&stream[off], sizeof(stream) - off, p1, 2);
    ASSERT_TRUE(n1 > 0); off += (size_t)n1;
    int n2 = fp_build(&stream[off], sizeof(stream) - off, p2, 5);
    ASSERT_TRUE(n2 > 0); off += (size_t)n2;

    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    fp_init(&fp, on_frame, &cap);

    ASSERT_EQ_SIZE(2u, fp_feed(&fp, stream, off));
    ASSERT_EQ_INT(2, cap.calls);
    ASSERT_EQ_INT(2, (int)fp_good_count(&fp));
    /* 最近一帧应是第二帧 */
    ASSERT_EQ_INT(5, cap.len);
    ASSERT_EQ_MEM(p2, cap.buf, 5);
    return 0;
}

/* ---- 坏 CRC：负载被篡改，应判为 CRC 错误，不交付 ---- */

static int test_crc_error(void) {
    const uint8_t payload[4] = {'A', 'B', 'C', 'D'};
    uint8_t frame[16];
    int flen = fp_build(frame, sizeof(frame), payload, 4);
    frame[3] ^= 0xFF;   /* 篡改一个负载字节，但 CRC 字段保持不变 */

    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    fp_init(&fp, on_frame, &cap);

    ASSERT_EQ_SIZE(0u, fp_feed(&fp, frame, (size_t)flen));
    ASSERT_EQ_INT(0, cap.calls);
    ASSERT_EQ_INT(0, (int)fp_good_count(&fp));
    ASSERT_EQ_INT(1, (int)fp_crc_error_count(&fp));
    return 0;
}

/* ---- 坏 LEN：长度超限应判为帧错误 ---- */

static int test_bad_len(void) {
    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    fp_init(&fp, on_frame, &cap);

    uint8_t bad[2] = { FP_STX, (uint8_t)(FP_MAX_PAYLOAD + 1) };
    ASSERT_EQ_SIZE(0u, fp_feed(&fp, bad, 2));
    ASSERT_EQ_INT(1, (int)fp_frame_error_count(&fp));
    ASSERT_EQ_INT(0, cap.calls);

    /* 之后还能正常解析下一帧（状态机已复位回 WAIT_STX） */
    const uint8_t payload[1] = {0x55};
    uint8_t frame[8];
    int flen = fp_build(frame, sizeof(frame), payload, 1);
    ASSERT_EQ_SIZE(1u, fp_feed(&fp, frame, (size_t)flen));
    ASSERT_EQ_INT(1, cap.calls);
    return 0;
}

/* ---- 满负载帧（FP_MAX_PAYLOAD 字节）往返 ---- */

static int test_max_payload(void) {
    uint8_t payload[FP_MAX_PAYLOAD];
    for (int i = 0; i < FP_MAX_PAYLOAD; i++) payload[i] = (uint8_t)(i * 7 + 1);

    uint8_t frame[FP_MAX_PAYLOAD + 5];
    int flen = fp_build(frame, sizeof(frame), payload, FP_MAX_PAYLOAD);
    ASSERT_EQ_INT(FP_MAX_PAYLOAD + 5, flen);

    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    fp_init(&fp, on_frame, &cap);
    ASSERT_EQ_SIZE(1u, fp_feed(&fp, frame, (size_t)flen));
    ASSERT_EQ_INT(FP_MAX_PAYLOAD, cap.len);
    ASSERT_EQ_MEM(payload, cap.buf, FP_MAX_PAYLOAD);
    return 0;
}

/* ---- 坏 ETX：CRC 正确但结束符错误，记帧错误并能恢复 ---- */

static int test_bad_etx(void) {
    const uint8_t payload[3] = {9, 8, 7};
    uint8_t frame[16];
    int flen = fp_build(frame, sizeof(frame), payload, 3);
    ASSERT_TRUE(flen > 0);
    frame[flen - 1] = 0x00;   /* 破坏 ETX（应为 FP_ETX） */

    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    fp_init(&fp, on_frame, &cap);

    ASSERT_EQ_SIZE(0u, fp_feed(&fp, frame, (size_t)flen));
    ASSERT_EQ_INT(0, cap.calls);
    ASSERT_EQ_INT(1, (int)fp_frame_error_count(&fp));

    /* 之后正常帧仍能解析 */
    uint8_t good[16];
    int glen = fp_build(good, sizeof(good), payload, 3);
    ASSERT_EQ_SIZE(1u, fp_feed(&fp, good, (size_t)glen));
    ASSERT_EQ_INT(1, cap.calls);
    return 0;
}

/* ---- CRC 错误后继续喂入好帧，解析器自我恢复 ---- */

static int test_crc_error_then_recover(void) {
    const uint8_t p[4] = {'A', 'B', 'C', 'D'};
    uint8_t bad[16];
    int blen = fp_build(bad, sizeof(bad), p, 4);
    bad[3] ^= 0xFF;   /* 篡改负载 -> CRC 不匹配 */

    uint8_t good[16];
    int glen = fp_build(good, sizeof(good), p, 4);

    /* 把坏帧 + 好帧拼成一个流一次喂入 */
    uint8_t stream[32];
    memcpy(stream, bad, (size_t)blen);
    memcpy(stream + blen, good, (size_t)glen);

    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    fp_init(&fp, on_frame, &cap);

    ASSERT_EQ_SIZE(1u, fp_feed(&fp, stream, (size_t)(blen + glen)));
    ASSERT_EQ_INT(1, cap.calls);                 /* 只有好帧被交付 */
    ASSERT_EQ_INT(1, (int)fp_good_count(&fp));
    ASSERT_EQ_INT(1, (int)fp_crc_error_count(&fp));
    ASSERT_EQ_MEM(p, cap.buf, 4);
    return 0;
}

/* ---- 状态机推进：逐字节喂入时 fp_state 应按序前进 ---- */

static int test_state_progression(void) {
    const uint8_t payload[2] = {0x5A, 0xA5};
    uint8_t frame[16];
    int flen = fp_build(frame, sizeof(frame), payload, 2);
    ASSERT_TRUE(flen > 0);

    frame_parser_t fp;
    fp_init(&fp, NULL, NULL);
    ASSERT_EQ_INT(FP_WAIT_STX, fp_state(&fp));      /* 初始 */

    fp_feed(&fp, &frame[0], 1);                     /* STX */
    ASSERT_EQ_INT(FP_WAIT_LEN, fp_state(&fp));
    fp_feed(&fp, &frame[1], 1);                     /* LEN */
    ASSERT_EQ_INT(FP_WAIT_PAYLOAD, fp_state(&fp));

    /* 喂完整帧后回到等待 STX */
    fp_feed(&fp, &frame[2], (size_t)(flen - 2));
    ASSERT_EQ_INT(FP_WAIT_STX, fp_state(&fp));

    /* reset 回到初始态 */
    fp_feed(&fp, &frame[0], 2);
    ASSERT_TRUE(fp_state(&fp) != FP_WAIT_STX);
    fp_reset(&fp);
    ASSERT_EQ_INT(FP_WAIT_STX, fp_state(&fp));
    return 0;
}

/* ---- 零负载帧 ---- */

static int test_zero_payload(void) {
    uint8_t frame[8];
    int flen = fp_build(frame, sizeof(frame), NULL, 0);
    ASSERT_EQ_INT(5, flen);

    cap_t cap = {{0}, 0, 0};
    frame_parser_t fp;
    fp_init(&fp, on_frame, &cap);
    ASSERT_EQ_SIZE(1u, fp_feed(&fp, frame, (size_t)flen));
    ASSERT_EQ_INT(1, cap.calls);
    ASSERT_EQ_INT(0, cap.len);
    return 0;
}

/* ---- 错误参数 ---- */

static int test_param_errors(void) {
    const uint8_t payload[4] = {1, 2, 3, 4};
    uint8_t small[6];
    /* out 容量不足（需要 9） */
    ASSERT_EQ_INT(-1, fp_build(small, sizeof(small), payload, 4));
    /* len 超限 */
    uint8_t big[80];
    ASSERT_EQ_INT(-1, fp_build(big, sizeof(big), payload, FP_MAX_PAYLOAD + 1));
    /* NULL 安全 */
    ASSERT_EQ_INT(-1, fp_init(NULL, NULL, NULL));
    ASSERT_EQ_SIZE(0u, fp_feed(NULL, payload, 4));
    ASSERT_EQ_INT(0, (int)fp_good_count(NULL));
    ASSERT_EQ_INT(FP_WAIT_STX, fp_state(NULL));
    fp_reset(NULL);
    return 0;
}

int main(void) {
    TEST_BEGIN();
    RUN_TEST(test_crc_known_vector);
    RUN_TEST(test_build_and_parse);
    RUN_TEST(test_byte_by_byte);
    RUN_TEST(test_two_frames_with_garbage);
    RUN_TEST(test_crc_error);
    RUN_TEST(test_crc_error_then_recover);
    RUN_TEST(test_bad_len);
    RUN_TEST(test_bad_etx);
    RUN_TEST(test_max_payload);
    RUN_TEST(test_state_progression);
    RUN_TEST(test_zero_payload);
    RUN_TEST(test_param_errors);
    TEST_END();
}
