/**
 * MQTT 业务链路层
 * 封装 Paho MQTT C 库，提供简洁的连接/订阅/发布接口
 */

#include <stdio.h>
#include "mqtt_client.h"

/* TODO: 集成 Paho MQTT C 库实现 */

int mqtt_client_init(const char *broker_ip, int port, const char *client_id) {
    (void)broker_ip; (void)port; (void)client_id;
    return 0;
}

int mqtt_client_subscribe(const char *topic, mqtt_msg_callback cb) {
    (void)topic; (void)cb;
    return 0;
}

int mqtt_client_publish(const char *topic, const char *payload) {
    (void)topic; (void)payload;
    return 0;
}

void mqtt_client_loop(void) {
}

void mqtt_client_destroy(void) {
}
