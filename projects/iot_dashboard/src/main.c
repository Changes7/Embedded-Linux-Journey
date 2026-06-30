/**
 * IoT Dashboard - 阶段 2 测试入口
 * 验证：JSON 解析 → dashboard_state → OLED 显示
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "oled_drv.h"
#include "dashboard_state.h"
#include "json_parser.h"

/* 模拟 MQTT 收到的控制报文 */
static const char *test_payload =
    "{\"temperature\":26.5,\"humidity\":58,\"led\":1,\"message\":\"Hello IoT\"}";

int main(void) {
    printf("[dashboard] 阶段 2: JSON → OLED 端到端测试\n");

    /* 初始化共享状态 */
    dashboard_state_t state;
    if (dashboard_state_init(&state) != 0) {
        fprintf(stderr, "[FATAL] 状态初始化失败\n");
        return EXIT_FAILURE;
    }

    /* 解析 JSON */
    printf("[dashboard] 解析 payload: %s\n", test_payload);
    if (json_parse_control(test_payload, &state) != 0) {
        fprintf(stderr, "[FATAL] JSON 解析失败\n");
        dashboard_state_destroy(&state);
        return EXIT_FAILURE;
    }
    printf("[dashboard] 解析结果 -> temp=%.1f  humi=%.0f  led=%d  msg=%s\n",
           state.temperature, state.humidity, state.led_state, state.message);

    /* 序列化回 JSON 并打印（上报测试） */
    char status_json[256];
    if (json_build_status(&state, status_json, sizeof(status_json)) > 0)
        printf("[dashboard] 上报 JSON: %s\n", status_json);

    /* 初始化 OLED */
    if (oled_init() < 0) {
        fprintf(stderr, "[FATAL] OLED 初始化失败\n");
        dashboard_state_destroy(&state);
        return EXIT_FAILURE;
    }

    /* 构造显示内容并刷屏 */
    char line[22];

    oled_show_string(0, 0, "STM32MP157 IoT");

    snprintf(line, sizeof(line), "Temp: %.1f C", state.temperature);
    oled_show_string(0, 2, line);

    snprintf(line, sizeof(line), "Humi: %.0f %%", state.humidity);
    oled_show_string(0, 3, line);

    snprintf(line, sizeof(line), "LED : %s", state.led_state ? "ON " : "OFF");
    oled_show_string(0, 4, line);

    oled_show_string(0, 6, state.message);

    oled_refresh();
    printf("[dashboard] OLED 刷新完成，保持 5 秒\n");
    sleep(5);

    oled_clear();
    dashboard_state_destroy(&state);
    printf("[dashboard] 测试完成\n");
    return EXIT_SUCCESS;
}
