#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

typedef void (*mqtt_msg_callback)(const char *topic, const char *payload, int len);

int mqtt_client_init(const char *broker_ip, int port, const char *client_id);
int mqtt_client_subscribe(const char *topic, mqtt_msg_callback cb);
int mqtt_client_publish(const char *topic, const char *payload);
void mqtt_client_loop(void);
void mqtt_client_destroy(void);

#endif
