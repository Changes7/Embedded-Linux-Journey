/**
 * cJSON 解析与封装层
 * 将 MQTT 下发的 JSON 字符串解析到 dashboard_state_t
 * 将设备状态序列化为 JSON 供上报使用
 */

#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "json_parser.h"

int json_parse_control(const char *payload, dashboard_state_t *state) {
    if (!payload || !state) return -1;

    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        fprintf(stderr, "[json] 解析失败: %s\n", cJSON_GetErrorPtr());
        return -1;
    }

    pthread_mutex_lock(&state->lock);

    cJSON *item;

    item = cJSON_GetObjectItemCaseSensitive(root, "temperature");
    if (cJSON_IsNumber(item))
        state->temperature = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(root, "humidity");
    if (cJSON_IsNumber(item))
        state->humidity = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(root, "led");
    if (cJSON_IsNumber(item))
        state->led_state = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(root, "message");
    if (cJSON_IsString(item) && item->valuestring)
        snprintf(state->message, sizeof(state->message), "%s", item->valuestring);

    pthread_mutex_unlock(&state->lock);

    cJSON_Delete(root);
    return 0;
}

int json_build_status(const dashboard_state_t *state, char *buf, int buf_size) {
    if (!state || !buf) return -1;

    pthread_mutex_lock((pthread_mutex_t *)&state->lock);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "temperature", state->temperature);
    cJSON_AddNumberToObject(root, "humidity",    state->humidity);
    cJSON_AddNumberToObject(root, "led",         state->led_state);
    cJSON_AddStringToObject(root, "message",     state->message);

    char *json_str = cJSON_PrintUnformatted(root);
    int ret = -1;
    if (json_str) {
        ret = snprintf(buf, buf_size, "%s", json_str);
        cJSON_free(json_str);
    }

    cJSON_Delete(root);
    pthread_mutex_unlock((pthread_mutex_t *)&state->lock);
    return ret;
}
