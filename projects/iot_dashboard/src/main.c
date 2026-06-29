/**
 * IoT Dashboard - 多线程核心协调调度器
 * 负责初始化各模块并协调 MQTT 接收线程与 OLED 刷新线程
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "oled_drv.h"
#include "mqtt_client.h"
#include "dashboard_state.h"

static volatile int g_running = 1;

static void sig_handler(int sig) {
    (void)sig;
    g_running = 0;
}

int main(void) {
    /* TODO: 初始化各模块，启动线程 */
    signal(SIGINT, sig_handler);

    printf("[dashboard] IoT Dashboard starting...\n");

    return EXIT_SUCCESS;
}
