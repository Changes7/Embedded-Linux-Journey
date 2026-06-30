#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "dashboard_state.h"

/**
 * 从 MQTT payload 解析控制指令，填充 state
 * 期望格式：{"temperature":26.5,"humidity":58,"led":1,"message":"Hello"}
 * 返回值：0 成功，-1 JSON 格式错误
 */
int json_parse_control(const char *payload, dashboard_state_t *state);

/**
 * 将 state 序列化为 JSON 字符串（用于上报）
 * 返回值：写入 buf 的字节数，-1 失败
 */
int json_build_status(const dashboard_state_t *state, char *buf, int buf_size);

#endif
