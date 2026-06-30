/**
 * 全局共享状态初始化与销毁
 */

#include <stdio.h>
#include <string.h>
#include "dashboard_state.h"

int dashboard_state_init(dashboard_state_t *state) {
    if (!state) return -1;
    memset(state, 0, sizeof(*state));
    state->temperature = 0.0f;
    state->humidity    = 0.0f;
    state->led_state   = 0;
    snprintf(state->message, sizeof(state->message), "WAIT");
    return pthread_mutex_init(&state->lock, NULL);
}

void dashboard_state_destroy(dashboard_state_t *state) {
    if (state)
        pthread_mutex_destroy(&state->lock);
}
