#ifndef DASHBOARD_STATE_H
#define DASHBOARD_STATE_H

#include <pthread.h>

typedef struct {
    float temperature;
    float humidity;
    int led_state;
    char message[64];
    pthread_mutex_t lock;
} dashboard_state_t;

int dashboard_state_init(dashboard_state_t *state);
void dashboard_state_destroy(dashboard_state_t *state);

#endif
