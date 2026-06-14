/**
 * @file frame_parser.h
 * @brief Lab 10 - 协议帧解析 + CRC16 API 声明。
 *
 * 工业背景
 * --------
 * 串口 / TCP 都是"字节流"，没有天然的消息边界。上位机连续发两条命令，
 * 接收端一次 read 可能同时读到一条半（粘包），也可能一条被拆成几次到达
 * （分包）。所以工业协议都要自己定义**帧格式**并做**帧同步**：靠起始符
 * 找到帧头、按长度收齐、用 **CRC 校验** 确认没传错，最后认 ETX 收尾。
 *
 * 本关定义一个简单帧：
 *     [STX=0x02][LEN][PAYLOAD(LEN字节)][CRC16_LO][CRC16_HI][ETX=0x03]
 *   - LEN：负载字节数（0 ~ FP_MAX_PAYLOAD）。
 *   - CRC16：对 PAYLOAD 这 LEN 个字节做 CRC-16/MODBUS，小端存放。
 *
 * 解析器用**字节流状态机**（呼应 Lab5）逐字节推进，遇到完整且校验通过的
 * 帧就回调交付。这样无论字节怎么分包/粘包，都能稳定地切出一帧帧。
 */
#ifndef LAB10_FRAME_PARSER_H
#define LAB10_FRAME_PARSER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 帧定界符与负载上限。 */
#define FP_STX          0x02
#define FP_ETX          0x03
#define FP_MAX_PAYLOAD  64

/**
 * CRC-16/MODBUS：初值 0xFFFF，多项式 0xA001（反射）。
 * @return data[0..len-1] 的 CRC16；data 为 NULL 或 len==0 时返回 0xFFFF（初值）。
 */
uint16_t crc16_modbus(const uint8_t *data, size_t len);

/**
 * 帧交付回调：解析出一帧合法数据时被调用。
 * @param payload 指向负载（解析器内部缓冲，回调结束后不保证有效，需要就拷走）。
 * @param len     负载长度。
 * @param ctx     用户上下文。
 */
typedef void (*fp_frame_cb)(const uint8_t *payload, uint8_t len, void *ctx);

/** 字节流状态机的状态。 */
typedef enum {
    FP_WAIT_STX = 0,   /**< 等待起始符         */
    FP_WAIT_LEN,       /**< 读长度             */
    FP_WAIT_PAYLOAD,   /**< 读负载             */
    FP_WAIT_CRC_LO,    /**< 读 CRC 低字节      */
    FP_WAIT_CRC_HI,    /**< 读 CRC 高字节      */
    FP_WAIT_ETX        /**< 等待结束符         */
} fp_state_t;

/**
 * 帧解析器控制块。
 * 字段公开仅为方便测试与教学，正常使用请只通过 API 访问。
 */
typedef struct {
    fp_state_t  state;                      /**< 当前状态           */
    uint8_t     payload[FP_MAX_PAYLOAD];    /**< 负载缓冲           */
    uint8_t     len;                        /**< 期望负载长度       */
    uint8_t     idx;                        /**< 已收负载字节数     */
    uint16_t    crc_rx;                     /**< 收到的 CRC         */
    fp_frame_cb cb;                         /**< 帧交付回调         */
    void       *ctx;                        /**< 回调上下文         */
    uint32_t    good_frames;                /**< 累计成功帧数       */
    uint32_t    crc_errors;                 /**< 累计 CRC 错误数    */
    uint32_t    frame_errors;               /**< 累计帧格式错误数（坏 LEN/ETX） */
} frame_parser_t;

/**
 * 初始化解析器。
 * @return 0 成功；-1（fp 为 NULL）。cb 可为 NULL（只统计不交付）。
 */
int fp_init(frame_parser_t *fp, fp_frame_cb cb, void *ctx);

/** 复位状态机与计数器（保留 cb/ctx）。fp 为 NULL 时安全返回。 */
void fp_reset(frame_parser_t *fp);

/**
 * 喂入一段字节流，逐字节驱动状态机。
 * 每解析出一帧校验通过的帧，就调用一次 cb，并累加 good_frames。
 * @return 本次调用交付的完整帧数；fp/data 为 NULL 时返回 0。
 */
size_t fp_feed(frame_parser_t *fp, const uint8_t *data, size_t len);

/** @return 累计成功帧数；fp 为 NULL 时返回 0。 */
uint32_t fp_good_count(const frame_parser_t *fp);

/** @return 累计 CRC 错误帧数；fp 为 NULL 时返回 0。 */
uint32_t fp_crc_error_count(const frame_parser_t *fp);

/** @return 累计帧格式错误数；fp 为 NULL 时返回 0。 */
uint32_t fp_frame_error_count(const frame_parser_t *fp);

/**
 * 按帧格式打包：把 payload 封装成一帧写入 out。
 * @param out      输出缓冲。
 * @param out_cap  out 容量。
 * @param payload  负载（可为 NULL 当 len==0）。
 * @param len      负载长度（<= FP_MAX_PAYLOAD）。
 * @return 整帧字节数（= len + 5）；参数非法或缓冲不够时返回 -1。
 */
int fp_build(uint8_t *out, size_t out_cap, const uint8_t *payload, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* LAB10_FRAME_PARSER_H */
